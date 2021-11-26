#include "custom_feature_def.h"
#include "ql_common.h"
#include "ql_system.h"
#include "ql_type.h"
#include "ql_gprs.h"
#include "ql_timer.h"
#include "ql_fota.h"
#include "ql_fs.h"
#include "ql_error.h"
#include "ril.h"
#include "ril_util.h"
#include "ril_ftp.h"
#include "ril_network.h"
#include "fota_ftp.h"
#include "fota_main.h"


#ifdef __OCPU_FOTA_BY_FTP__
#define FTP_CONNECT_ATTEMPTS        (5)     // max 5 times for attempt to connect, or restart module
static u8 Contextid;
static s32 SerPort;
static u8 ServerAdder[FTP_SERVERADD_LEN] = {0x0}; // ip string or domain name
static u8 FilePath[FTP_FILEPATH_LEN] = {0x0};  // file in the server path
static u8 Ftp_userName[FTP_USERNAME_LEN] = {0x0};
static u8 Ftp_Possword[FTP_PASSWORD_LEN] = {0x0};
static u8 appBin_fName[FTP_BINFILENAME_LEN] = {0x0};

extern ST_FotaConfig    FotaConfig;
extern u8 Fota_apn[MAX_GPRS_APN_LEN];
extern u8 Fota_userid[MAX_GPRS_USER_NAME_LEN];
extern u8 Fota_passwd[MAX_GPRS_PASSWORD_LEN];
extern CallBack_Ftp_Download FtpGet_IND_CB;
extern Callback_Upgrade_State Fota_UpgardeState;


static void FTP_Program(void);

static void DoUpgrade(void);
static bool FTP_DecodeURL(u8 *URL, s32 URLlength, u8 *serverAdd, u8* filePath,  u8* binFile, u8* ftpUserName, u8 *ftpUserPassword, s32* serverPort);

s32 FTP_FotaMain(u8 contextId, u8* URL)
{
    s32 retValue;
    bool ftpDecodeURL;

    APP_DEBUG("<--Fota ftp Main entry !-->\r\n");

    ftpDecodeURL = FTP_DecodeURL(URL, Ql_strlen((char*)URL), ServerAdder, FilePath, appBin_fName, Ftp_userName, Ftp_Possword, &SerPort);
    if(!ftpDecodeURL)
    {
        APP_DEBUG("<--ftp  DECODE URL FAILED !!!-->\r\n");
        return -1;
    }
    Contextid = contextId;

    FTP_Program();

    return 0;
}

bool FTP_IsFtpServer(u8* URL)
{
	APP_DEBUG("FTP_IsFtpServer");
    char buffer[6];
    Ql_memset(buffer, 0x0, sizeof(buffer));

    Ql_memcpy(buffer, URL, 5);
    Ql_StrToUpper(buffer);

    if(NULL == Ql_strstr(buffer, "FTP"))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void FTP_Callback_OnDownload(s32 result,s32 size)
{
    s32 ret;
    bool retValue;
    if (result)
    {
    	APP_DEBUG("<---image bin file size =%d--->\r\n",size);
        FOTA_UPGRADE_IND(UP_GET_FILE_OK,100,retValue);
    }else{
    	APP_DEBUG("<-- FTP fails to get file, cause=%d -->\r\n", size);
        FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
    }
    ret = RIL_FTP_QFTPCLOSE();
    APP_DEBUG("<-- FTP close connection, ret=%d -->\r\n", ret);

    APP_DEBUG("<-- Deactivating PDP context -->\r\n");

    ret = RIL_FTP_QIDEACT();
    APP_DEBUG("<-- Released PDP context, ret=%d -->\r\n", ret);

    // Start to do upgrading by FOTA
    if (result)
    {
    #if 1
        DoUpgrade();
    #else
        // Don't do FOTA, just do FTP repeatedly (for test)
        Ql_OS_SendMessage(main_task_id, MSG_ID_FTP_RESULT_IND, FTP_RESULT_SUCCEED, 0);
    #endif
    }
    else
    {
        Ql_OS_SendMessage(main_task_id, MSG_ID_FTP_RESULT_IND, FTP_RESULT_FAILED, 0);
    }
}

static void FTP_Program(void)
{
    s32 ret;
    u8  attempts = 0;
    bool retValue;
    u32 fileSize = 0;
    
    ret = RIL_NW_SetGPRSContext(Contextid);
    APP_DEBUG( "<-- Set GPRS PDP context, ret=%d -->\r\n", ret);

    ret = RIL_NW_SetAPN(1,(char*)Fota_apn,(char*)Fota_userid,(char*)Fota_passwd);
    APP_DEBUG("<-- Set GPRS APN, ret=%d, apn=[%s] userid=[%s] passwd=[%s]-->\r\n", ret, Fota_apn, Fota_userid, Fota_passwd);

    FOTA_UPGRADE_IND(UP_CONNECTING, 0, retValue);
    do
    {
        ret = RIL_FTP_QFTPOPEN(ServerAdder, SerPort, Ftp_userName, Ftp_Possword, 1);
        APP_DEBUG("<-- FTP open connection, ret=%d -->\r\n", ret);
        if (RIL_AT_SUCCESS == ret)
        {
            attempts = 0;
            FOTA_UPGRADE_IND(UP_CONNECTED,0,retValue);
            break;
        }
        attempts++;
        Ql_Sleep(2000);
    } while (attempts < FTP_CONNECT_ATTEMPTS);
    if (FTP_CONNECT_ATTEMPTS == attempts)
    {
        // Infor the caller to reset the module
        Ql_OS_SendMessage(main_task_id, MSG_ID_RESET_MODULE_REQ, attempts, ret);
        return;
    }

    ret = RIL_FTP_QFTPCFG(4, (u8*)APP_BINFILE_PATH); 
    APP_DEBUG("<-- Set local storage, ret=%d -->\r\n", ret);

    ret = RIL_FTP_QFTPPATH(FilePath);   
    APP_DEBUG("<-- Set remote path, ret=%d -->\r\n", ret);

    ret = RIL_FTP_QFTPSIZE(appBin_fName,&fileSize);
    APP_DEBUG("<-- Get file Size, ret=%d,fileSize=%d -->\r\n", ret,fileSize);

    FOTA_UPGRADE_IND(UP_GETTING_FILE,0,retValue);
    // By default, 3min time out in RIL
    ret = RIL_FTP_QFTPGET(appBin_fName, fileSize, FTP_Callback_OnDownload);
    APP_DEBUG("<-- Downloading FTP file, ret=%d -->\r\n", ret);
    if (ret < 0)
    {
        ret = RIL_FTP_QFTPCLOSE();
        APP_DEBUG( "<-- FTP close connection, ret=%d -->\r\n", ret);

        ret = RIL_FTP_QIDEACT();
        APP_DEBUG("<-- Release PDP context, ret=%d -->\r\n", ret);

        // Inform the caller of FTP downloading failed
        Ql_OS_SendMessage(main_task_id, MSG_ID_FTP_RESULT_IND, FTP_RESULT_FAILED, ret);
    }
}

void DoUpgrade(void)
{
    u32 filesize,fd_file;
    u8 *file_buffer=NULL;
    s32 ret2,ret3;
    u32 realLen,lenToRead;
    bool retValue;
    u8 binfilePath[FTP_BINFILENAME_LEN+4];

    // Prepare update data
    if (!Ql_strncmp(APP_BINFILE_PATH,"UFS", 3))//UFS
    {
        Ql_sprintf((char*)binfilePath,"%s",(char*)appBin_fName);
    }
    else
    {
        Ql_sprintf((char*)binfilePath,"%s:%s",APP_BINFILE_PATH,(char*)appBin_fName);
    }

    ret2 = Ql_FS_Check((char*)binfilePath);
    if (QL_RET_OK == ret2)
    {
        filesize = Ql_FS_GetSize((char*)binfilePath);
        if(filesize < 0)
        {
            APP_DEBUG("\r\n<-- Fail to get size (App)  Failed ret =%d-->\r\n",filesize);
            FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
            return;
        }

        if (Ql_strncmp((char*)binfilePath,"RAM:", 4))
        {
            fd_file = Ql_FS_Open((char*)binfilePath, QL_FS_READ_ONLY);
        }
        else
        {
            fd_file = Ql_FS_OpenRAMFile((char*)binfilePath, QL_FS_READ_ONLY, filesize);
        }

        if(fd_file < 0)
        {
            APP_DEBUG("\r\n<-- Fail to open (App) !!-->\n");
            FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
            return ;
        }
        file_buffer = Ql_MEM_Alloc(UP_DATA_BUFFER_LEN);
        if(NULL ==file_buffer)
        {
            APP_DEBUG("\r\n<-- fota ftp  memory allocation failed !!-->\r\n");
            FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
            return ;
        }
        /*Write App  bin to flash*/  
        APP_DEBUG( "<-- Processing data before update -->\r\n");
        while(filesize>0)
        {
            if (filesize <= UP_DATA_BUFFER_LEN)
            {
                lenToRead = filesize;
            } else {
                lenToRead = UP_DATA_BUFFER_LEN;
            }
            ret3 = Ql_FS_Read(fd_file, file_buffer, lenToRead, &realLen);
            if(ret3 != 0)
            {
                APP_DEBUG("<--Ql_FS_Read failed(ret =%d)-->\r\n",ret3);
                FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
                return ;
            }
            //   UPGRADE_APP_DEBUG(FOTA_DBGBuffer,"<--  lenToRead %d  ret=%d-->\r\n", lenToRead,ret3);
            ret3 = Ql_FOTA_WriteData(realLen,(s8*)file_buffer);
            if(ret3 != 0)
            {
                APP_DEBUG( "<--Ql_FOTA_WriteData failed(ret =%d)-->\r\n",ret3);
                FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
                return ;
            }
            filesize -= realLen;
            //  UPGRADE_APP_DEBUG(FOTA_DBGBuffer,"<--  lenToRead %d Fota write, len=%d, ret=%d, filesize=%d -->\r\n", lenToRead,realLen, ret3, filesize);
            //Ql_Sleep(10);
        }
        if(NULL != file_buffer)
        {
            Ql_MEM_Free(file_buffer);
            file_buffer = NULL;
        }
        APP_DEBUG("<-- Finish processing data -->\r\n");
        Ql_FS_Close(fd_file);
        APP_DEBUG("<-- Close file -->\r\n");
    }
    else //if (QL_RET_ERR_FILENOTFOUND == ret2)
    {
        APP_DEBUG("\r\n<-- App Bin(%s) does not exsit  ret=%d-->\r\n", (char*)binfilePath,ret2);
        FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
        return;      
    }
    Ql_Sleep(300);
    ret3 = Ql_FOTA_Finish();     //Finish the upgrade operation ending with calling this API
    if(ret3 != 0)
    {
        APP_DEBUG("\r\n<-- Fota App Finish Fail!(ret =%d) -->\r\n",ret3);
        FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
        return;
    }

    Ql_FS_Delete((char*)binfilePath);
    APP_DEBUG("<-- Delete file -->\r\n");
    APP_DEBUG("<-- Start to Update! If you return TRUE in the fota upgrade callback in the UP_SYSTEM_REBOOT case,  the module will automatically restart.-->\r\n");

    // Start to upgrade
    FOTA_UPGRADE_IND(UP_SYSTEM_REBOOT,100,retValue);
    if(NULL == Fota_UpgardeState  || (retValue))//  if fota upgrade callback function return TRUE in the UP_SYSTEM_REBOOT case ,the system upgrade app at once.
    {
        APP_DEBUG("<--Fota upgrade callback return TRUE!!, system reboot for upgrade now-->\r\n");

        Ql_Sleep(300);
        ret3=Ql_FOTA_Update();
        if(0 != ret3)
        {
            APP_DEBUG("\r\n<-- Ql_Fota_Update FAILED!ret3=%d -->\r\n",ret3);
            APP_DEBUG("\r\n<-- Reboot 1 second later ... -->\r\n");
            FOTA_UPGRADE_IND(UP_UPGRADFAILED,0,retValue);
            Ql_Sleep(1000);
            //Ql_Reset(0);
        }
        else
        {
            //If update OK, module will reboot automatically
        }
    }
    else
    {
        // if  upgrade state callback function return false, you must call Ql_FOTA_Update function  before you reboot the system
    }
}

bool FTP_DecodeURL(u8 *URL, s32 URLlength, u8 *serverAdd, u8* filePath,  u8* binFile, u8* ftpUserName, u8 *ftpUserPassword, s32* serverPort)
{
    u8  hstr[7];
    u32 j,i;
    u8 *phostnamehead;
    u8 *phostnameTail;
    char portStr[8];
    bool ret = FALSE;
    u8 *puri=URL;
    u32 datalen = URLlength;
    u32 filePathLen,binfilenameLen;
    *serverPort = FTP_SERVICE_PORT;

    do
    {    
        /* Resolve ftp:// */
        Ql_memset(hstr,0,7);

        if((datalen) < 7)
            break;
        Ql_memcpy(hstr,URL,7);
        for(i=0;i<6;i++)
        {
            if( (hstr[i]>= 'A') && (hstr[i] <= 'Z'))
                hstr[i] = Ql_tolower(hstr[i]);
        }
        if(NULL != Ql_strstr((char *)hstr,"ftp://"))
        {
            puri = URL + 6;
            datalen -= 6;
        }
        else
        {
            break;
        }
        i=0;
        /* retrieve host name */
        phostnamehead = puri;
        phostnameTail = puri;
        while(i<datalen && puri[i] != '/' && puri[i] != ':')
        {
            i++;
            phostnameTail++;
        }
        if(i > FTP_SERVERADD_LEN)
        {
            APP_DEBUG("<--server Addess is too long!!! (the buffer limit size is %d)->\r\n",FTP_SERVERADD_LEN);
            break;
        }
        Ql_memcpy((char*)serverAdd,(char*)phostnamehead, i);  // here is server ip or domain name.
        // UPGRADE_APP_DEBUG(FOTA_DBGBuffer,"<--!!serverAdd=%s-->\r\n",serverAdd);

        if(datalen >= i)
            datalen -= i;
        else
            break;


        /* retrieve  file path ,  image file name and ,port      eg /filepath/file/xxx.bin:8021@user......  or   /filepath/file/xxx.bin@user......  */
        puri+=i;
        i = 0;
        phostnamehead = puri;
        phostnameTail = puri;
        while(puri[i] !=':' && puri[i] !='@' && i<datalen )
        {
            i++;
            phostnameTail++;
        }		
        if(datalen >= i)
        {
            datalen -= i;
        }
        else // no @username:password     eg :  ftp://192.168.10.10/file/test.bin 
        {
            j = i;
            while(puri[j] !='/' && j>0) // the last '/' char 
            {
                j--;
            }
            binfilenameLen = (i-j-1);
            filePathLen = i-(i-j);
            if((binfilenameLen > FTP_BINFILENAME_LEN) ||(filePathLen > FTP_FILEPATH_LEN))
            {
                APP_DEBUG("<--!! bin file name len(%d) or filePath(lne %d) is to loog ! limit len(binfilename=%d, filePath=%d)->\r\n",binfilenameLen,filePathLen,FTP_BINFILENAME_LEN,FTP_FILEPATH_LEN);
                break;
            }
            Ql_memcpy((char *)binFile, (char *)(puri+j+1),binfilenameLen);
            Ql_memcpy((char *)filePath, (char *)phostnamehead, filePathLen);
            break;
        }

        /* retrieve  file path ,  image file name  /filepath/file/xxx.bin@user......  */
        if(puri[i] =='@' ) // no port number , it means port number is 21
        {
            j = i;
            while(puri[j] !='/' && j>0) // the last '/' char 
            {
                j--;
            }
            binfilenameLen = (i-j-1);
            filePathLen = i-(i-j);
            if((binfilenameLen > FTP_BINFILENAME_LEN) ||(filePathLen > FTP_FILEPATH_LEN))
            {
                APP_DEBUG("<--@@ bin file name len(%d) or filePath(lne %d) is to loog ! limit len(binfilename=%d, filePath=%d)->\r\n",binfilenameLen,filePathLen,FTP_BINFILENAME_LEN,FTP_FILEPATH_LEN);
                break;
            }
            Ql_memcpy((char *)binFile, (char *)(puri+j+1),binfilenameLen);
            Ql_memcpy((char *)filePath, (char *)phostnamehead, filePathLen); 
            APP_DEBUG("<--@@binFile=%s filePath=%s datalen=%d-->\r\n",binFile,filePath,datalen);
        }
        /* retrieve file path , image file name and port   /filepath/file/xxx.bin:8021@user......  */
        else// else if (puri[i] ==':')     ftp port number.  
        {
            j = i;
            while(puri[j] !='/' && j>0) // the last '/' char 
            {
                j--;
            }
            binfilenameLen = (i-j-1);
            filePathLen = i-(i-j);
            if((binfilenameLen > FTP_BINFILENAME_LEN) ||(filePathLen > FTP_FILEPATH_LEN))
            {
                APP_DEBUG("<--## bin file name len(%d) or filePath(lne %d) is to loog ! limt len(binfilename=%d, filePath=%d)->\r\n",binfilenameLen,filePathLen,FTP_BINFILENAME_LEN,FTP_FILEPATH_LEN);
                break;
            }
            Ql_memcpy((char *)binFile, (char *)(puri+j+1),binfilenameLen);
            Ql_memcpy((char *)filePath, (char *)phostnamehead, filePathLen); 

            puri+=i; // puri = :port@username....
            i = 0;
            phostnamehead = puri;
            phostnameTail = puri;
            while(i<datalen && puri[i] != '@' )
            {
                i++;
                phostnameTail++;
            }
            if(datalen >= i)
                datalen -= i;
            else
                break;
            Ql_memset(portStr, 0x00,sizeof(portStr));
            Ql_memcpy(portStr, phostnamehead+1, i-1);
            *serverPort =  Ql_atoi(portStr);
            //   UPGRADE_APP_DEBUG(FOTA_DBGBuffer,"<--&&portStr=%s server port=%d-->\r\n",portStr,*serverPort);    
        }

        /* retrieve the ftp username and password      eg  @username:password  */
        puri+=i;
        i = 0;
        phostnamehead = puri;
        phostnameTail = puri;
        while(puri[i] !=':' && i<datalen )
        {
            i++;
            phostnameTail++;
        }
        if(datalen >= i)
            datalen -= i;
        else
            break;
        if(0 == datalen)// no user password
        {
            if(i > FTP_USERNAME_LEN)
            {
                APP_DEBUG("<--@@ ftp user name(len %d) is to loog ! limit len(%d)->\r\n",i,FTP_USERNAME_LEN);
                break;
            }
            Ql_memcpy(ftpUserName,phostnamehead+1,i); // ftp user name          
        }
        else
        {
            if((i-1 > FTP_USERNAME_LEN) || (datalen > FTP_PASSWORD_LEN))
            {
                APP_DEBUG("<--@@ ftp user name(len %d)  or password(len=%d)is to loog ! limit len(username=%d, pwd =%d)->\r\n",i-1,datalen,FTP_USERNAME_LEN,FTP_PASSWORD_LEN);
                break;
            }
            Ql_memcpy(ftpUserName,phostnamehead+1,i-1); // ftp user name 
            Ql_memcpy(ftpUserPassword,phostnamehead+i+1,datalen); //user  password
        }

        ret =TRUE;
    }while(FALSE);
	
	if(*(filePath+Ql_strlen((const char*)filePath)-1) != '/'){
        Ql_memcpy((void*)(filePath+Ql_strlen((const char*)filePath)), (const void*)"/", 1);  //  file path end with '/'
	}
    //Ql_memcpy((void*)(filePath+Ql_strlen((const char*)filePath)), (const void*)"/", 1);  //  file path end with '/'
    APP_DEBUG("<--serverAdd=%s, file path=%s, image filename=%s-->\r\n",ServerAdder,FilePath,appBin_fName);
    APP_DEBUG("<--ftp user name=%s, user password =%s  serverPort=%d-->\r\n",Ftp_userName, Ftp_Possword,*serverPort);
    return ret;
}

#endif  //__OCPU_FOTA_BY_FTP__
