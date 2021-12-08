/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2021
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ql_fs.h 
 *
 * Project:
 * --------
 *   QuecOpen
 *
 * Description:
 * ------------
 *   File  API defines.
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
 

#ifndef __QL_FS_H__
#define __QL_FS_H__
#include "ql_type.h"

/****************************************************************************
 * Type of file access permitted
 ***************************************************************************/
typedef enum
{
	QL_READ_WRITE = 0,
	QL_READ_WRITE_CREATE_ALWAYS,
	QL_READ_ONLY,

}Enum_FsOpenFlag;

/****************************************************************************
 * Constants for File Seek
 ***************************************************************************/
typedef enum
{
   QL_FS_FILE_BEGIN,    // Beginning of file
   QL_FS_FILE_CURRENT,  // Current position of file pointer
   QL_FS_FILE_END       // End of file
}Enum_FsSeekPos;



/******************************************************************************
* Function:     Ql_FS_Open
*  
* Description:
*               Opens or creates a file with a specified name in the UFS. 
*               If you want to create a file in the UFS , you only need to use a relative path.
*
* Parameters:    
*               lpFileName:
*                   [in]The name of the file. The name is limited to 252 characters.
*                       You must use a relative path, such as "filename.ext" or 
*                       "dirname\filename.ext". 

*
*               flag:
*                   [in]A u32 that defines the file's opening and access mode.
*                       The possible values are shown as follow:
*                       QL_FS_READ_WRITE, can read and write
*                       QL_FS_READ_ONLY, can only read
*                       QL_FS_CREATE, opens the file, if it exists. 
*                           If the file does not exist, the function creates the file
*                       QL_FS_CREATE_ALWAYS, creates a new file. 
*                           If the file exists, the function overwrites the file 
*                           and clears the existing attributes
* Return:  
*               If the function succeeds, the return value specifies a file handle.
*               If the function fails, the return value is an error codes. 
*               QL_RET_ERR_PARAM indicates parameter error. 
*               QL_RET_ERR_FILEOPENFAILED indicates open file failed.
******************************************************************************/
s32 Ql_FS_Open(char* lpFileName, Enum_FsOpenFlag flag);

/******************************************************************************
* Function:     Ql_FS_Read
*  
* Description:
*               Reads data from the specified file, starting at the position 
*               indicated by the file pointer. After the read operation has been 
*               completed, the file pointer is adjusted by the number of bytes actually read.
*
* Parameters:    
*               fileHandle:
*                   [in] A handle to the file to be read, which is the return value
*                        of the function Ql_FS_Open.
*
*               readBuffer:
*                   [out] Point to the buffer that receives the data read from the file.
*
*               numberOfBytesToRead:
*                   [in] Number of bytes to be read from the file.
*
*               numberOfBytesRead:
*                   [out] The number of bytes has been read. sets this value to zero before
*                         doing taking action or checking errors.
* Return:
*               QL_RET_OK, suceess
*               QL_RET_ERR_PARAM, parameter error.
*               QL_RET_ERR_FILE_STATE , file state error.
*               QL_RET_ERR_FILEREADFAILED , read file failed.
******************************************************************************/
s32 Ql_FS_Read(s32 fileHandle, u8 *readBuffer, u32 numberOfBytesToRead, u32 *numberOfBytesRead);


/******************************************************************************
* Function:     Ql_FS_Write
*  
* Description:
*               This function writes data to a file. Ql_FS_Write starts writing 
*               data to the file at the position indicated by the file pointer.
*               After the write operation has been completed, the file pointer 
*               is adjusted by the number of bytes actually written. 
*
* Parameters:    
*               fileHandle:
*                   [in] A handle to the file to be read, which is the return value 
*                        of the function Ql_FS_Open.
*
*               writeBuffer:
*                   [in] Point to the buffer containing the data to be written to the file.
*
*               numberOfBytesToWrite:
*                   [in] Number of bytes to be write to the file.
*
*               numberOfBytesWritten:
*                   [out]  The number of bytes has been written. Sets this value to zero 
*                        before doing taking action or checking errors.
* Return:  
*               QL_RET_OK, suceess
*               QL_RET_ERR_PARAM,parameter error.
*               QL_RET_ERR_FILE_STATE , file state error.
*               QL_RET_ERR_FILEWRITEFAILED, write file failed. 
******************************************************************************/
s32 Ql_FS_Write(s32 fileHandle, u8 *writeBuffer, u32 numberOfBytesToWrite, u32 *numberOfBytesWritten);


/******************************************************************************
* Function:     Ql_FS_Seek
*  
* Description:
*               Repositions the pointer in the previously opened file. 
*
* Parameters:    
*               fileHandle:
*                   [in] A handle to the file to be read, which is the return value 
*                        of the function Ql_FS_Open.
*
*               offset:
*                   [in] Number of bytes to move the file pointer.
*
*               whence:
*                   [in] The file pointer reference position. See Enum_FsSeekPos.
* Return:  
*               QL_RET_OK, suceess
*               QL_RET_ERR_FILE_STATE , file state error.
*               QL_RET_ERR_FILESEEKFAILED, file seek failed
******************************************************************************/
s32 Ql_FS_Seek(s32 fileHandle, s32 offset, u32 whence);


/******************************************************************************
* Function:     Ql_FS_GetFilePosition
*  
* Description:
*               Gets the current value of the file pointer.
*
* Parameters:    
*               fileHandle:
*                   [in] A file handle, which was returned by calling 'Ql_FS_Open'.
*
* Return:  
*               The return value is the current offset from the beginning of the file
*               if this function succeeds. Otherwise, the return value is an error code. 
*               QL_RET_ERR_FILEFAILED, fail to operate file.
******************************************************************************/
s32 Ql_FS_GetFilePosition(s32 fileHandle);


/******************************************************************************
* Function:     Ql_FS_Flush
*  
* Description:
*               Forces any data remaining in the file buffer to be written to the file.
*
* Parameters:    
*               fileHandle: 
*                   [in] A file handle, which was returned by calling 'Ql_FS_Open'.
* Return:  
*               None
******************************************************************************/
void Ql_FS_Flush(s32 fileHandle);


/******************************************************************************
* Function:     Ql_FS_Close
*  
* Description:
*               Closes the file associated with the file handle and makes 
*               the file unavailable for reading or writing.
*
* Parameters:    
*               fileHandle: 
*                   [in] A file handle, which was returned by calling 'Ql_FS_Open'.
* Return:  
*               QL_RET_OK, success.
*               QL_RET_ERR_PARAM, parameter error.
*               QL_RET_ERR_FILE_STATE , file state error.
*               QL_RET_ERR_FILEFAILED, fail to operate file. 
******************************************************************************/
s32 Ql_FS_Close(s32 fileHandle);


/******************************************************************************
* Function:     Ql_FS_GetSize
*  
* Description:
*               Retrieves the size, in bytes, of the specified file.
*
* Parameters:    
*               fileHandle:
*                   [in] A file handle, which was returned by calling 'Ql_FS_Open'.
*
* Return:  
*               The return value is the bytes of the file if this function succeeds. 
*               Otherwise, the return value is an error code. 
*               QL_RET_ERR_FILEFAILED, fail to operate file.
******************************************************************************/
s32 Ql_FS_GetSize(s32 fileHandle);


/******************************************************************************
* Function:     Ql_FS_Delete
*  
* Description:
*               This function deletes an existing file.
*
* Parameters:    
*               lpFileName:
*                   [in]The name of the file to be deleted. The name is limited 
*                       to 252 characters. You must use a relative path, such as 
*                       "filename.ext" or "dirname/filename.ext".
* Return:  
*               QL_RET_OK, success.
*               QL_RET_ERR_PARAM, parameter error. 
*               QL_RET_ERR_FILE_STATE , file state error.
*               QL_RET_ERR_FILEFAILED, fail to operate file.
******************************************************************************/
s32 Ql_FS_Delete(char *lpFileName);


/******************************************************************************
* Function:     Ql_FS_Check
*  
* Description:
*               Check whether the file exists or not.
*
* Parameters:    
*               lpFileName:
*                   [in] The name of the file. The name is limited to 252 characters.
*                        You must use a relative path, such as "filename.ext" or 
*                        "dirname/filename.ext".
* Return:  
*               QL_RET_OK, success.
*               QL_RET_ERR_PARAM, parameter error. 
*               QL_RET_ERR_FILENOTFOUND, file not found.
******************************************************************************/
s32 Ql_FS_Check(char *lpFileName);


/******************************************************************************
* Function:     Ql_FS_GetFreeSpace
*  
* Description:
*               This function obtains the amount of free space on Flash.
*
* Parameters:    
*               None.
*
* Return:  
*               The return value is the total number of bytes of the free space 
*               in the specified storage, if this function succeeds. Otherwise, 
*               the return value is an error code.
*               QL_RET_ERR_FILEFAILED,get space error.
******************************************************************************/
s32  Ql_FS_GetFreeSpace (void);


/******************************************************************************
* Function:     Ql_FS_GetTotalSpace
*  
* Description:
*               This function obtains the amount of total space on Flash.
*
* Parameters:    
*               None.
*
* Return:  
*               The return value is the total number of bytes in the specified  
*               storage, if this function succeeds. Otherwise, the return value
*               is an error code.
*               QL_RET_ERR_FILEFAILED,get space error.
******************************************************************************/
s32  Ql_FS_GetTotalSpace(void);


/******************************************************************************
* Function:     Ql_Fs_Format  
*                           
* Description:
*               This function format the UFS
*           
* Parameters:    
*               None.
*
* Return:       
*               QL_RET_OK, success.
*               QL_RET_ERR_PARAM, parameter error. 
******************************************************************************/
s32 Ql_FS_Format(void);

#endif  //__QL_FS_H__

