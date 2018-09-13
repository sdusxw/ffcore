/*********************************************************************************
  *Copyright(C):        sdboon.com
  *FileName:			boon_log.h
  *Author:				diaoguangqiang
  *Version:             2.0
  *Date:                2017.10.15
  *Description:			日志管理类
  *History:             (修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介)
     1. Date:           2017.10.15
        Author:			diaoguangqiang
        Modification:   首次生成文件
     2. Data:
        Author:
        Modification:
**********************************************************************************/
#ifndef BASE_LOG_H
#define BASE_LOG_H

#include <stdio.h>
//#include <string>
#include <string.h>
#include <vector>
#include <fstream>
#include <stdlib.h>

//
#if defined(WIN32)
#include <direct.h>
#include <io.h>
#else
//
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#endif

#include <time.h>

#define UNIX
#if defined(UNIX)
#include <unistd.h>
#endif

using namespace std;

//
#if defined(WIN32)
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#else
#define ACCESS access
#define MKDIR(a) mkdir((a), 0775)
#endif

//#define LOG_ROOT_NAME "log"
//#define BOON_LOG_ROOT "/home/boon/log"

/*
#ifndef SYS_LOG
#define LOG_DIR_NAME "data_log"
#define LOG_FILE_NAME "log"
#endif // SYS_LOG
*/

#define CREATE_LOG_HOUR
#define CREATE_LOG_TIME	30
//---
namespace BASE{

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#ifndef VERSION_10
#define VERSION_10
#endif

//--
enum LOG_TYPE{	
	STATUS = 1,	//
	PKGDATA = 2,//
	PKGEROR = 3,//
	
};

//--
enum RET_CODE{
	CODE_SUCCESS = 0,	
	CODE_POINT = 1,		// 
	CODE_GROUP = 2,		// 
	CODE_LINK  = 3,		//  
	CODE_FILE = 4,		//  
	CODE_TYPE = 5,		// ?
	CODE_CREATE_FEP_LOG_EOR = 6,// ??
	CODE_CREATE_GROUP_DIR_EOR = 7,//  
	CODE_MKDIR_FEP_EOR = 8,	//MK 
	CODE_MKDIR_GROUP_EOR = 9,//MK 
};

class BLog{
	public:

		static int writelog( const void* _buf, int _type, const void* _groupName, const void* _linkName)
		{
            /*
#if defined(DEBUG)
#else
            return 0;
#endif
             */
			if ( !_groupName )	return CODE_GROUP;
			if ( !_linkName )	return CODE_LINK;
			if ( !_buf	)		return CODE_POINT;

			int ret = CODE_SUCCESS;
			switch( _type )
			{
				case STATUS:
					ret = write_link_status_log( _groupName, _linkName, _buf );
					break;

				case PKGDATA:
					ret = write_link_pkg_log( _groupName, _linkName, _buf );
					break;

				case PKGEROR:
					ret = write_link_eror_log( _groupName, _linkName, _buf );
					break;

				default:
					ret = CODE_TYPE;
					break;
			}

			return ret;
		}


	private:
		static int write_link_status_log( const void* _groupName, const void* _linkName, const void* _buf )
		{
			if ( !_groupName )	return CODE_GROUP;
			if ( !_linkName )	return CODE_LINK;
			if ( !_buf	)		return CODE_POINT;
						
			int ret = CODE_SUCCESS;
			std::string content = (char*)_buf;
			std::string groupName = (char*)_groupName;
						
			char fep_log_dir[MAX_PATH] = {0};
			if ( CODE_SUCCESS != ( ret = createLogDir( fep_log_dir, _groupName ) ) )
				return ret;
			
			std::string strdir = fep_log_dir;
			std::string filename= strdir + getfilename( _linkName, STATUS ) + ".log";

#ifdef VERSION_10
			FILE *file = fopen(filename.c_str(), "a+");
			if (file == NULL) return CODE_FILE;
#else
			FILE *file;
			if (fopen_s(&file, filename.c_str(), "a+") != 0)
			{
				return CODE_FILE;
			}
#endif // VERSION_10
			
			std::string curcontent;
			if ( "" == content )
				curcontent = content;
			else
				curcontent = gettimex() + " " + content;

			fwrite( curcontent.c_str(), sizeof(char), curcontent.size(), file );
			
			fputs( "\n", file );
			fclose(file);

			return CODE_SUCCESS;
		}


		static int write_link_pkg_log( const void* _groupName, const void* _linkName, const void* _buf )
		{
			if ( !_groupName )	return CODE_GROUP;
			if ( !_linkName )	return CODE_LINK;
			if ( !_buf	)		return CODE_POINT;

			std::string content = (char*)_buf;
			std::string groupName = (char*)_groupName;

			int ret = CODE_SUCCESS;
			char fep_log_dir[MAX_PATH] = {0};
			if ( CODE_SUCCESS != ( ret = createLogDir( fep_log_dir, _groupName ) ) )
				return ret;

			std::string strdir = fep_log_dir;
			std::string filename= strdir + getfilename( _linkName, PKGDATA ) + ".log";
            //printf("[%s] filename:%s [%d]\n", __FUNCTION__, filename.c_str(), __LINE__);

#ifdef VERSION_10
			FILE *file = fopen(filename.c_str(), "a+");
			if (file == NULL) return CODE_FILE;
#else
			FILE *file;
			if (fopen_s(&file, filename.c_str(), "a+") != 0)
			{
				return CODE_FILE;
			}
#endif // VERSION_10

			std::string curcontent;
			if ( "" == content )
				curcontent = content;
			else
				curcontent = gettimex() + " " + content;
			
			fwrite( curcontent.c_str(), sizeof(char), curcontent.size(), file );

			fputs( "\n", file );
			fclose(file);

			return CODE_SUCCESS;
		}

		static int write_link_eror_log( const void* _groupName, const void* _linkName, const void* _buf )
		{
			if ( !_groupName )	return CODE_GROUP;
			if ( !_linkName )	return CODE_LINK;
			if ( !_buf	)		return CODE_POINT;

			std::string content = (char*)_buf;
			std::string groupName = (char*)_groupName;

			int ret = CODE_SUCCESS;
			char fep_log_dir[MAX_PATH] = {0};
			if ( CODE_SUCCESS != ( ret = createLogDir( fep_log_dir, _groupName ) ) )
				return ret;

			std::string strdir = fep_log_dir;
			std::string filename= strdir + getfilename( _linkName, PKGEROR ) + ".log";

#ifdef VERSION_10
			FILE *file = fopen(filename.c_str(), "a+");
			if (file == NULL) return CODE_FILE;
#else
			FILE *file;
			if (fopen_s(&file, filename.c_str(), "a+") != 0)
			{
				return CODE_FILE;
			}
#endif // VERSION_10

			std::string curcontent;
			if ( "" == content )
				curcontent = content;
			else
				curcontent = gettimex() + " " + content;

			fwrite( curcontent.c_str(), sizeof(char), curcontent.size(), file );

			fputs( "\n", file );
			fclose(file);

			return CODE_SUCCESS;
		}

		static string getfilename( const void* _linkName, int _type )
		{
			if ( !_linkName )	return "errlink";

			std::string strLinkName = (char*)_linkName;
			
			time_t t = time(0);
			
			char buf_file_name[64] = {0};

	#ifdef VERSION_10
			tm *pt = NULL;

			#ifdef UNIX
            t = time(NULL);
			#endif

			pt = localtime(&t);
	#else
			struct tm pt;
			localtime_s(&pt, &t);
	#endif // VERSION_10
									
			switch( _type )
			{
				case STATUS:
					{
#ifdef VERSION_10
					snprintf(buf_file_name, sizeof(buf_file_name), "%s_status_", strLinkName.c_str());
#else
					sprintf_s(buf_file_name, sizeof(buf_file_name), "%s_status_", strLinkName.c_str());
#endif // VERSION_10
						
						int len = strlen(buf_file_name);
#ifdef CREATE_LOG_HOUR
						strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H_", (tm*)&pt);					
#else
						if ( pt->tm_min < CREATE_LOG_TIME )
							strftime(buf+len, sizeof(buf)-len, "%Y%m%d%H00_", pt);	
						else
							strftime(buf+len, sizeof(buf)-len, "%Y%m%d%H30_", pt);	
#endif						
					}
					break;

				case PKGDATA:
					{
#ifdef VERSION_10
					    snprintf(buf_file_name, sizeof(buf_file_name), "%s_pakage_", strLinkName.c_str());
#else
					    sprintf_s(buf_file_name, sizeof(buf_file_name), "%s_pakage_", strLinkName.c_str());
#endif // VERSION_10

						int len = strlen(buf_file_name);
#ifdef CREATE_LOG_HOUR
#ifdef UNIX
                        strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H_", pt);
#else
                        strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H_", (tm*)&pt);
#endif
                        //printf("[%s] buf_file_name:%s [%d]\n", __FUNCTION__, buf_file_name+len, __LINE__);
#else
						if ( pt->tm_min < CREATE_LOG_TIME )
							strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H00_", pt);	
						else
							strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H30_", pt);

                        //printf("[%s] buf_file_name:%s [%d]\n", __FUNCTION__, buf_file_name, __LINE__);
#endif
						
					}
					break;

				//-- ---
				case PKGEROR:
					{
#ifdef VERSION_10
						snprintf(buf_file_name, sizeof(buf_file_name), "%s_errors_", strLinkName.c_str());
#else
						sprintf_s(buf_file_name, sizeof(buf_file_name), "%s_errors_", strLinkName.c_str());
#endif // VERSION_10

						int len = strlen(buf_file_name);
#ifdef CREATE_LOG_HOUR
						strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H_", (tm*)&pt);		
#else
						if ( pt->tm_min < CREATE_LOG_TIME )
							strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H00_", pt);	
						else
							strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H30_", pt);	
#endif
					}
					break;

				default:
					{
#ifdef VERSION_10
						snprintf(buf_file_name, sizeof(buf_file_name), "%s_error_", strLinkName.c_str());
#else
						sprintf_s(buf_file_name, sizeof(buf_file_name), "%s_error_", strLinkName.c_str());
#endif // VERSION_10
						int len = strlen(buf_file_name);
#ifdef CREATE_LOG_HOUR
						strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H_", (tm*)&pt);		
#else
						if ( pt->tm_min < CREATE_LOG_TIME )
							strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H00_", pt);	
						else
							strftime(buf_file_name+len, sizeof(buf_file_name)-len, "%Y%m%d%H30_", pt);	
#endif
						
					}
					break;
			}
			
			return string(buf_file_name);
		}


		static string gettime()
		{
			char buf_time[64] = {0};
#ifdef WIN32
			time_t t = time(0);
			tm *pt;

#ifdef VERSION_10
			pt = localtime(&t);
#else
			localtime_s(pt, &t);
#endif // VERSION_10

			strftime(buf_time, sizeof(buf_time), "[%Y-%m-%d %H:%M:%S]", pt);
#else
			struct timeval tv;  
			struct timezone tz;  
			struct tm *p;  

			gettimeofday(&tv, &tz);  
			//printf("tv_sec:%ld\n",tv.tv_sec);  
			//printf("tv_usec:%ld\n",tv.tv_usec);  
			//printf("tz_minuteswest:%d\n",tz.tz_minuteswest);  
			//printf("tz_dsttime:%d\n",tz.tz_dsttime);  

			p = localtime(&tv.tv_sec);  
			snprintf( buf_time, sizeof(buf_time), "[%04d-%02d-%02d %02d:%02d:%02d %03ld]",
				1900+p->tm_year, 1+p->tm_mon, p->tm_mday, 
				p->tm_hour, p->tm_min, p->tm_sec, 
				tv.tv_sec*1000+tv.tv_usec/1000 );  	
#endif			

			return string(buf_time);
		}


		static string gettimex(){		
			char buf[64] = {0};

#ifdef WIN32
			time_t t = time(0);
			
#ifdef VERSION_10
			tm *pt = NULL;
			pt = localtime(&t);
#else
			struct tm pt;
			localtime_s(&pt, &t);
#endif // VERSION_10
			strftime(buf, sizeof(buf), "[%Y-%m-%d %H:%M:%S]", (tm*)&pt);
#else
			struct timeval tms;
			char tmp[20] = {0};
			timerclear(&tms);
			gettimeofday(&tms,NULL);
			strftime( tmp, sizeof(tmp), "%X", localtime(&tms.tv_sec) );

			snprintf( buf, sizeof(buf), "[%s %03d]", tmp, static_cast<int>(tms.tv_usec/1000));
#endif
			return string( buf );
		}


		static int createLogDir(const char* _buf, const void* _groupName ){
			if ( !_groupName )	return CODE_GROUP;			
			if ( !_buf	)		return CODE_POINT;
			
			std::string groupName = (char*)_groupName;

#ifdef VERSION_10
			//snprintf((char*)_buf, MAX_PATH, "%s/%s/", getenv("HOME"), LOG_ROOT_NAME);
			snprintf((char*)_buf, MAX_PATH, "%s/", getenv("HOME")/*BOON_LOG_ROOT*/);
#else
			//char env_buf[MAX_PATH] = { 0 };
			char* env_buf = NULL;
			size_t env_size = MAX_PATH;
			_dupenv_s(&env_buf, &env_size, "SYS_ROOT");
			sprintf_s((char*)_buf, MAX_PATH, "%s/%s/", /*getenv("SYS_ROOT")*/ env_buf, LOG_ROOT_NAME);

#endif // VERSION_10

			//MKDIR( _buf );
			
#ifdef WIN32
			_mkdir( _buf );
#else
			if ( -1 == ACCESS(_buf, 0 ) )
				if ( -1 == mkdir( _buf, 0775 ) )
					return CODE_MKDIR_FEP_EOR;
#endif

			memset( (char*)_buf, 0, sizeof(_buf)/sizeof(_buf[0]) );

#ifdef VERSION_10
			//snprintf((char*)_buf, MAX_PATH, "%s/%s/%s/", getenv("HOME"), LOG_ROOT_NAME, groupName.c_str());
			snprintf((char*)_buf, MAX_PATH, "%s/%s/", getenv("HOME")/*BOON_LOG_ROOT*/, groupName.c_str());
#else
			env_buf = NULL;
			_dupenv_s(&env_buf, &env_size, "SYS_ROOT");
			sprintf_s((char*)_buf, MAX_PATH, "%s/%s/%s/", env_buf, LOG_ROOT_NAME, groupName.c_str());
#endif // VERSION_10
			
			//MKDIR(_buf);
			
#ifdef WIN32
			_mkdir( _buf );
#else
			if ( -1 == access( _buf, 0 ) )	
				if ( -1 == mkdir( _buf, 0775 ) )
					return CODE_MKDIR_GROUP_EOR;
#endif
			return CODE_SUCCESS;
		}

		private:
			enum{ BUFFER_SIZE_LOG = 3000 };		// ?
		
	};
	
}

/*
int main(int, char *[])
{
	using namespace FEP_LOG;
	
	char* pgroupName="Z18BAS";
	char* plinkName ="Z18BAS";
	char buf[]={"11223344"};
	
	while( 1 )
	{
		//-
		if ( !FepLog::writelog( pgroupName, plinkName, buf, STATUS ) )
		{
			printf("error! line[%d]", __LINE__);
			break;
		}

		//sleep(1000);

		//-
		if ( !FepLog::writelog( pgroupName, plinkName, buf, PKGDATA ) )
		{
			printf("error! line[%d]", __LINE__);
			break;
		}
	}
	
	return 0;
};
*/

#endif // BASE_LOG_H