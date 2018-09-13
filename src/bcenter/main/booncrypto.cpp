#include "booncrypto.h"
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/hdreg.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <string>
#include <fstream>
#include <math.h>
#define BUFFSIZE 1024

Booncrypto::Booncrypto()
{
}
Booncrypto::~Booncrypto()
{

}

bool Booncrypto::generatefile(char *path)
{
    std::ofstream file(path);
    int max = 128;
    char id[128] = {0};
    if(!getdiskid(id,max))
    {
        return false;
    }
    char dst[1024] = {0};
    if(!private_encrypt(id,dst,&max))
    {
        return false;
    }
    file.write(dst,max);
    file.close();
    return true;
}
bool Booncrypto::generatefile(char *path,char *keyfile)
{
    std::ofstream file(path);
    int max = 128;
    char id[128] = {0};
    if(!getdiskid(id,max))
    {
        return false;
    }
    char dst[1024] = {0};
    if(!private_encrypt(id,keyfile,dst,&max))
    {
        return false;
    }
    file.write(dst,max);
    file.close();
    return true;
}

bool Booncrypto::verifyfile(char *path)
{
    std::ifstream file;
    file.open(path,std::ios::in);
    if(!file)
    {
        return false;
    }
    char src[1024] = {0};
    file.read(src,1024);
    file.close();
    char *dst = (char *)malloc(128);
    memset(dst,0,128);
    int max = 0;
    if(!public_decrypt(src,dst,&max))
    {
        return false;
    }
    bool ret = isexistindir(dst,"/dev/disk/by-id");
    free(dst);
    return ret;
}
bool Booncrypto::verifyfile()
{
    bool ret = false;
    if(decrypt_keyfile("/home/boon/.key/pub_key","/tmp/.pub_key"))
    {
        ret = verifyfile("/home/boon/.key/key","/tmp/.pub_key");
        remove("/tmp/.pub_key");
    }
    return ret;
}

bool Booncrypto::verifyfile(char *path,char *keyfile)
{
    std::ifstream file;
    file.open(path,std::ios::in);
    if(!file)
    {
        return false;
    }
    char src[1024] = {0};
    file.read(src,1024);
    file.close();
    char *dst = (char *)malloc(128);
    memset(dst,0,128);
    int max = 0;
    if(!public_decrypt(src,keyfile,dst,&max))
    {
        return false;
    }
    //bool ret = isexistindir(dst,"/dev/disk/by-id");
    bool ret;
    char diskid[128] = {0};
    getdiskid(diskid,128);
    if(strcmp(dst,diskid) == 0)
        ret = true;
    else
        ret = false;
    free(dst);
    return ret;
}

const char* Booncrypto::get_public_key(int *len)
{
    static std::string strPublicKey = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDJ3aZ/yEF8e5w88aAgcLZbVyoRMv0MOmIotUOihTWh4p/cstZlWqQgr+82pvpbY6H1fgZoTltgjKEEPmkgc1URrbC/38R69Y7uzsrSd28PZssUDcuZtVeOj85aEJVv2zGOpjFi9X58dHLJClLyS13N09VT\qh+uvQxxc1MjVc9bqQIDAQAB";
    int nPublicKeyLen = strPublicKey.size();
    for(int i = 64; i < nPublicKeyLen; i+=64)
    {
        if(strPublicKey[i] != '\n')
        {
            strPublicKey.insert(i, "\n");
        }
        i++;
    }
    strPublicKey.insert(0, "-----BEGIN PUBLIC KEY-----\n");
    strPublicKey.append("\n-----END PUBLIC KEY-----\n");
    *len = strPublicKey.length();
    return const_cast<char *>(strPublicKey.c_str());
}
const char* Booncrypto::get_private_key(int *len)
{
    static std::string strPrivateKey = "MIICXAIBAAKBgQDJ3aZ/yEF8e5w88aAgcLZbVyoRMv0MOmIotUOihTWh4p/cstZlWqQgr+82pvpbY6H1fgZoTltgjKEEPmkgc1URrbC/38R69Y7uzsrSd28PZssUDcuZtVeOj85aEJVv2zGOpjFi9X58dHLJClLyS13N09VTqh+uvQxxc1MjVc9bqQIDAQABAoGASIHo8DUJ8KClue5ASeayWZSxc4QRCHdgEqcpKBMY9JDmQuupvrYUBfmrvsGzY2giIH+n5YdfowpgO5n/QLysbQ0VQqqglTiX3u9Knxi1H2maqB9ubX1DZzOLqJTmugixYM+snzOrXJnEqY+dUV+oOsrx9vwUB2aUrZplsEl7p9ECQQD6kLw3+pmMI/ENdXYKPdTxakCDXiDRlOTtluIuJx7WYY6MVGBa+ZhvGysbleCZPK/GQdcHJKh4iOiXtmMCoRt7AkEAzj6Cw8+fzOQ7Z4gZnf9VAfL4DvIYUnWTJ8Z/DlSTaCFfkrEhZhK+HqVQ4+tMoNeeWEITmrno9qWuMPfEujvaKwJABA51zZ40AC3QyDv/ljjcCrCCrN3IQDxd3G7V6JNfj27y5Ni02qQx0JKrBv5NLY3q9pW4SnhQdesZgONGBRPgowJBAKI+NJobI6eHx197hkNvUA0XeKIxOobrrRZ2JQ894zPgRRHdu9tTVTJAdDDHsmE5HXxqhoeKRkR5M12cG6sxWD8CQHxE/pm56hSDHR8qWb3MpRiffeMwt0d+fCnu+5JGXpv/qAqSTQcQrqS1J2bN7IQR07Rqqi9yGJYhTY7ALRlcTpo=";
    int nPrivateKeyLen = strPrivateKey.size();
    for(int i = 64; i < nPrivateKeyLen; i+=64)
    {
        if(strPrivateKey[i] != '\n')
        {
            strPrivateKey.insert(i, "\n");
        }
        i++;
    }
    strPrivateKey.insert(0, "-----BEGIN RSA PRIVATE KEY-----\n");
    strPrivateKey.append("\n-----END RSA PRIVATE KEY-----\n");
    *len = strPrivateKey.length();
    return const_cast<char *>(strPrivateKey.c_str());
}

bool Booncrypto::public_encrypt(char *src,char *dst,int *dst_len)
{
    BIO *bio = NULL;
    RSA *rsa = NULL;
    int length;
    const char *public_key = get_public_key(&length);
    if((bio = BIO_new_mem_buf((void *)public_key,length)) == NULL)
    {
        std::cout<<"BIO_new_mem_buf failed!"<<std::endl;
        return false;
    }
    if(!(rsa = PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL)))
    {
        std::cout<<"read key failed"<<std::endl;
        ERR_load_crypto_strings();
        char errBuf[512];
        ERR_error_string_n(ERR_get_error(), errBuf, sizeof(errBuf));
        std::cout<< "load public key failed["<<errBuf<<"]"<<std::endl;
        return false;
    }
    BIO_free_all(bio);
    int rsa_len=RSA_size(rsa);
    *dst_len = rsa_len+1;
    memset(dst,0,rsa_len+1);
    if(RSA_public_encrypt(rsa_len,(unsigned char *)src,(unsigned char*)dst,rsa,RSA_NO_PADDING)<0)
    {
        return false;
    }
    RSA_free(rsa);
    return true;
}

bool Booncrypto::public_decrypt(char *src,char *dst,int *dst_len)
{
    if(strlen(src) == 0)
        return false;
    BIO *bio = NULL;
    RSA *rsa = NULL;
    int length;
    const char *public_key = get_public_key(&length);
    if((bio = BIO_new_mem_buf((void *)public_key,length)) == NULL)
    {
        std::cout<<"BIO_new_mem_buf failed!"<<std::endl;
        return false;
    }
    if(!(rsa = PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL)))
    {
        std::cout<<"read key failed"<<std::endl;
        ERR_load_crypto_strings();
        char errBuf[512];
        ERR_error_string_n(ERR_get_error(), errBuf, sizeof(errBuf));
        std::cout<< "load public key failed["<<errBuf<<"]"<<std::endl;
        return false;
    }
    BIO_free_all(bio);
    int rsa_len=RSA_size(rsa);
    *dst_len = rsa_len+1;
    memset(dst,0,rsa_len+1);
    if(RSA_public_decrypt(rsa_len,(unsigned char *)src,(unsigned char*)dst,rsa,RSA_NO_PADDING)<0)
    {
        return false;
    }
    RSA_free(rsa);
    return true;
}

bool Booncrypto::private_encrypt(char *src,char *dst,int *dst_len)
{
    BIO *bio = NULL;
    RSA *rsa = NULL;
    int length;
    const char *private_key = get_private_key(&length);
    if((bio = BIO_new_mem_buf((void *)private_key,length)) == NULL)
    {
        std::cout<<"BIO_new_mem_buf failed!"<<std::endl;
        return false;
    }
    if(!(rsa = PEM_read_bio_RSAPrivateKey(bio,NULL,NULL,NULL)))
    {
        std::cout<<"read key failed"<<std::endl;
        ERR_load_crypto_strings();
        char errBuf[512];
        ERR_error_string_n(ERR_get_error(), errBuf, sizeof(errBuf));
        std::cout<< "load public key failed["<<errBuf<<"]"<<std::endl;
        return false;
    }
    BIO_free_all(bio);
    int rsa_len=RSA_size(rsa);
    *dst_len = rsa_len+1;
    memset(dst,0,rsa_len+1);
    if(RSA_private_encrypt(rsa_len,(unsigned char *)src,(unsigned char*)dst,rsa,RSA_NO_PADDING)<0)
    {
        return false;
    }
    RSA_free(rsa);
    return true;
}

bool Booncrypto::private_decrypt(char *src,char *dst,int *dst_len)
{
    BIO *bio = NULL;
    RSA *rsa = NULL;
    int length;
    const char *private_key = get_private_key(&length);
    if((bio = BIO_new_mem_buf((void *)private_key,length)) == NULL)
    {
        std::cout<<"BIO_new_mem_buf failed!"<<std::endl;
        return false;
    }
    if(!(rsa = PEM_read_bio_RSAPrivateKey(bio,NULL,NULL,NULL)))
    {
        std::cout<<"read key failed"<<std::endl;
        ERR_load_crypto_strings();
        char errBuf[512];
        ERR_error_string_n(ERR_get_error(), errBuf, sizeof(errBuf));
        std::cout<< "load public key failed["<<errBuf<<"]"<<std::endl;
        return false;
    }
    BIO_free_all(bio);
    int rsa_len=RSA_size(rsa);
    *dst_len = rsa_len+1;
    memset(dst,0,rsa_len+1);
    if(RSA_private_decrypt(rsa_len,(unsigned char *)src,(unsigned char*)dst,rsa,RSA_NO_PADDING)<0)
    {
        return false;
    }
    RSA_free(rsa);
    return true;
}

bool Booncrypto::public_encrypt(char *src,char *path_key,char *dst,int *dst_len)
{
    //get_public_key(NULL);
    char *p_en = dst;
    RSA *p_rsa;
    FILE *file;
    int flen,rsa_len;
    if((file=fopen(path_key,"r"))==NULL)
    {
        perror("open key file error");
        return false;
    }
    if((p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL))==NULL)
    {
        ERR_print_errors_fp(stdout);
        return false;
    }
    flen=strlen(src);
    rsa_len=RSA_size(p_rsa);
    //p_en=(char *)malloc(rsa_len+1);
    *dst_len = rsa_len+1;
    memset(p_en,0,rsa_len+1);
    if(RSA_public_encrypt(rsa_len,(unsigned char *)src,(unsigned char*)p_en,p_rsa,RSA_NO_PADDING)<0)
    {
        return false;
    }
    RSA_free(p_rsa);
    fclose(file);
    return true;
}
bool Booncrypto::public_decrypt(char *src,char *path_key,char *dst,int *dst_len)
{
    char *p_en = dst;
    RSA *p_rsa;
    FILE *file;
    int flen,rsa_len;
    if((file=fopen(path_key,"r"))==NULL)
    {
        perror("open key file error");
        return false;
    }
    if((p_rsa=PEM_read_RSA_PUBKEY(file,NULL,NULL,NULL))==NULL)
    {
        ERR_print_errors_fp(stdout);
        return false;
    }
    flen=strlen(src);
    rsa_len=RSA_size(p_rsa);
    //p_en=(char *)malloc(rsa_len+1);
    *dst_len = rsa_len+1;
    memset(p_en,0,rsa_len+1);
    if(RSA_public_decrypt(rsa_len,(unsigned char *)src,(unsigned char*)p_en,p_rsa,RSA_NO_PADDING)<0)
    {
        return false;
    }
    RSA_free(p_rsa);
    fclose(file);
    return true;
}
bool Booncrypto::private_encrypt(char *src,char *path_key,char *dst,int *dst_len)
{
    char *p_de = dst;
    RSA *p_rsa;
    FILE *file;
    int rsa_len;
    if((file=fopen(path_key,"r"))==NULL)
    {
        perror("open key file error");
        return false;
    }
    if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL)
    {
        ERR_print_errors_fp(stdout);
        return false;
    }
    rsa_len=RSA_size(p_rsa);
    //p_de=(char *)malloc(rsa_len+1);
    *dst_len = rsa_len + 1;
    memset(p_de,0,rsa_len+1);
    if(RSA_private_encrypt(rsa_len,(unsigned char *)src,(unsigned char*)p_de,p_rsa,RSA_NO_PADDING)<0)
    {
        return false;
    }
    RSA_free(p_rsa);
    fclose(file);
    return true;
}

bool Booncrypto::private_decrypt(char *src,char *path_key,char *dst,int *dst_len)
{
    char *p_de = dst;
    RSA *p_rsa;
    FILE *file;
    int rsa_len;
    if((file=fopen(path_key,"r"))==NULL)
    {
        perror("open key file error");
        return false;
    }
    if((p_rsa=PEM_read_RSAPrivateKey(file,NULL,NULL,NULL))==NULL)
    {
        ERR_print_errors_fp(stdout);
        return false;
    }
    rsa_len=RSA_size(p_rsa);
    //p_de=(char *)malloc(rsa_len+1);
    *dst_len = rsa_len + 1;
    memset(p_de,0,rsa_len+1);
    if(RSA_private_decrypt(rsa_len,(unsigned char *)src,(unsigned char*)p_de,p_rsa,RSA_NO_PADDING)<0)
    {
        return false;
    }
    RSA_free(p_rsa);
    fclose(file);
    return true;
}
bool Booncrypto::getdiskid(char *id,int max)
{
    int fd;
    struct hd_driveid hid;
    fd = open ("/dev/sda", O_RDONLY);
    if (fd < 0)
    {
        fprintf (stderr, "open hard disk device failed.\n");
        return false;
    }
    if (ioctl (fd, HDIO_GET_IDENTITY, &hid) < 0)
    {
        fprintf (stderr, "ioctl error./n");
        return false;
    }
    close (fd);
    snprintf (id, max, "%s",hid.serial_no);
    printf("%s\n",id);
    std::ofstream out("/home/boon/.key/SerialNo");
    out.write(id,strlen(id));
    out.close();
    return true;
}
int Booncrypto::getfileinode(char *filename)
{
    struct stat buf = {0};
    if(stat(filename,&buf) == -1)
    {
        printf("stat error!\n");
        return 0;
    }
    return buf.st_ino;
}
bool Booncrypto::isexistindir(char *str,char *dir)
{
    if(strlen(str) == 0)
        return false;
    std::string name = str;
    DIR *pdir = opendir(dir);
    if(!pdir)
    {
        printf("open dir %s error\n",dir);
        return false;
    }
    struct dirent *dirp;
    while((dirp = readdir(pdir))!=NULL)
    {
       std::string filename = dirp->d_name;
       if(filename.find(name) != -1)
       {
           return true;
       }
    }
    return false;
}

bool	Booncrypto::signalfunc(int n)
{
    static double middle = sin(M_PI_4);
    return sin(n) > middle;
}

char* 	Booncrypto::encrypt_key(char *src,int length)
{
    for(int i = 0;i<length;i++)
    {
        if(src[i] != '\n')
            src[i] += (signalfunc(i) ? 1 : 0);
    }
    return src;
}
char* 	Booncrypto::decrypt_key(char *src,int length)
{
    for(int i = 0;i<length;i++)
    {
        if(src[i] != '\n')
            src[i] -= (signalfunc(i) ? 1 : 0);
    }
    return src;

}

bool Booncrypto::encrypt_keyfile(char *path,char *newpath)
{
    char buf[128] = {0};
    std::ifstream in;
    std::ofstream out;
    in.open(path);
    if(!in)
    {
        return false;
    }
    out.open(newpath);
    while(!in.eof())
    {
        in.getline(buf,128);
        if(buf[0] != '-')
        {
            encrypt_key(buf,strlen(buf));
        }
        out.write(buf,strlen(buf));
        out.write("\n",1);
        memset(buf,0,128);
    }
    in.close();
    out.close();
}

bool Booncrypto::decrypt_keyfile(char *path,char *newpath)
{
    char buf[128] = {0};
    std::ifstream in;
    std::ofstream out;
    in.open(path);
    if(!in)
    {
        return false;
    }
    out.open(newpath);
    while(!in.eof())
    {
        in.getline(buf,128);
        if(buf[0] != '-')
        {
            decrypt_key(buf,strlen(buf));
        }
        out.write(buf,strlen(buf));
        out.write("\n",1);
        memset(buf,0,128);
    }
    in.close();
    out.close();
}
