#ifndef BOONCRYPTO_H
#define BOONCRYPTO_H


class Booncrypto
{

public:
    Booncrypto();
    ~Booncrypto();
    bool generatefile(char *path);		//产生授权文件
    bool generatefile(char *path,char *keyfile);		//产生授权文件
    bool verifyfile();
    bool verifyfile(char *path);		//验证授权文件
    bool verifyfile(char *path,char *keyfile);		//验证授权文件
    bool encrypt_keyfile(char *path,char *newpath);
    bool decrypt_keyfile(char *path,char *newpath);
private:
    const char* 	get_public_key(int *len);
    const char* 	get_private_key(int *len);

    bool	signalfunc(int n);
    char* 	encrypt_key(char *src,int length);				//加密字符串
    char* 	decrypt_key(char *src,int length);				//解密字符串
    bool	public_encrypt(char *src,char *path_key,char *dst,int *dst_len);//公钥加密
    bool	public_decrypt(char *src,char *path_key,char *dst,int *dst_len);//公钥解密
    bool 	private_encrypt(char *src,char *path_key,char *dst,int *dst_len);//私钥加密
    bool 	private_decrypt(char *src,char *path_key,char *dst,int *dst_len);//私钥解密
    bool	public_encrypt(char *src,char *dst,int *dst_len);//公钥加密
    bool	public_decrypt(char *src,char *dst,int *dst_len);//公钥解密
    bool 	private_encrypt(char *src,char *dst,int *dst_len);//私钥加密
    bool 	private_decrypt(char *src,char *dst,int *dst_len);//私钥解密
    bool 	getdiskid(char *id,int max); 	//获取硬盘序列号
    int 	getfileinode(char *filename);	//获取文件inode
    bool 	isexistindir(char *str,char *dir);		//判断目录下是否包含文件名
};

#endif // BOONCRYPTO_H
