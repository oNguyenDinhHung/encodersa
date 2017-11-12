#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rsa.h"
#include "consant.h"
#include "time.h"
int luy_thua_cao(int x,int y,int mod){// tính x mũ y modul mod
	int a,b,tmp;
	if(y==0) return 1%mod;
	else
		if(y==1) return x%mod;
		else {
			a=y/2;
			tmp=(x*x)%mod;
			if(y%2==0)
				return luy_thua_cao(tmp,a,mod);
			else{
				tmp=luy_thua_cao(tmp,a,mod);
				return (tmp*x)%mod;
			}
		}	
}
int* gcd_mo_rong(int n1,int n2){
	register int a1=1,b2=1,a2=0,b1=0,q,r,t;
	int *result;
	result=(int*)malloc(3*sizeof(int));// result là mảng trả về chứa g,a,b
	do{
		q=n1/n2,r=n1%n2;
		if(r==0){
			result[0]=n2,result[1]=a2,result[2]=b2;			
			return result;		
		}
		else{
			n1=n2,n2=r,t=a2,a2=a1-q*a2,a1=t,t=b2,b2=b1-q*b2,b1=t;
		}
	}while(r!=0);

}
void init(rsa_params *x){
	srand(time(NULL));
	//vòng lặp để chọn ra p và q:
	do{
		x->p=primes[rand()%50],	x->q=primes[rand()%50];// nếu chọn p, q to quá thì n sẽ to và khi tính Y^d bị lòi
	}while(x->p==x->q||x->p*x->q<256);	
	x->n=x->p*x->q,x->m=(x->p-1)*(x->q-1);

	//vòng lặp để chọn ra e:
	do{
		x->e=primes[rand()%1000];
	}while(x->e>x->m-1||x->m%x->e==0);

	//dùng thuật toán gcd mở rộng để tìm ra d:
	int *tmp;
	tmp=gcd_mo_rong(x->e,x->m);
	if(tmp[1]<0) x->d=tmp[1]+x->m;// nếu d bị âm ta phải + với m để thành dương.
	else x->d=tmp[1];	
}
int rsa_encode(int x,rsa_params _rsa){
	return luy_thua_cao(x,_rsa.e,_rsa.n);
}
int rsa_decode(int y,rsa_params _rsa){
	return luy_thua_cao(y,_rsa.d,_rsa.n);
}
void ma_hoa(rsa_params _rsa,char* fname){
	memset(&sbuff,0,sizeof(buffer));
	memset(&dbuff,0,sizeof(buffer));
	int u=0;
	while(2<<u++ < _rsa.n-1);//tính số bit mỗi lần mã hóa, phải thóa 2^u < n-1.
	printf("mỗi lần mã hóa %d bit nhé\n",num_of_bit= u-1);	
	sf=fopen(fname,"rb");
	df=fopen("encode","wb");	
	while(!feof(sf))
		export_from_des_buffer();			
	fclose(sf);
	fclose(df);
}
void giai_ma(rsa_params _rsa,char* fname){
	FILE *f1,*f2;
	char plain,code;
	f1=fopen(fname,"rb");
	f2=fopen("decode","wb");
	if(!f1||!f2) puts("IO error !!!");
	while(!feof(f1)){
		if(fread(&code,sizeof(char),1,f1)>0){
			plain=rsa_decode(code,_rsa);
			printf("%d->%d ",code,plain );
			fwrite(&plain,sizeof(char),1,f2);
		}		
	}	
	fclose(f1),fclose(f2);
}

extern buffer sbuff,dbuff;//sbuff là buffer của file nguồn,dbuff là buffer của file đích.
extern int num_of_bit;// số lượng bit của mỗi khối sẽ đem đi mã hóa bằng RSA.
extern FILE *sf,*df;
int import_to_src_buffer(){// hàm này thực hiện đọc một byte dữ liệu rồi chèn nó vào sau src buffer-data.
	char c;
	if(sbuff.pos+BYTE_LEN < BUFFER_LEN && !feof(sf)){		
		if(fread(&c,sizeof(char),1,sf)>0){
			sbuff.pos+=sizeof(char)<<3;
			sbuff.data=(sbuff.data<<BYTE_LEN)+c;			
			return 1;
		}
		else{
			puts("End of file!");
			return 0;
		}
	}
}
unsigned int export_from_src_buffer(){// hàm này trả về (num_of_bit) đầu tiên của file f.
	unsigned int result=MAX_VAL, tmp=(2<<num_of_bit)-1;// tạo ra một số tmp gồm (num_of_bit) các bit 1
	while(sbuff.pos<num_of_bit){
		if(import_to_src_buffer(sf,sbuff)==0){	// nếu hết file thì ta tự chèn thêm các bit 0 vào data cho đủ số bit.	
			if(sbuff.pos==0) break;	
			sbuff.data= sbuff.data<<(num_of_bit-sbuff.pos);
			sbuff.pos= num_of_bit;					
		}
	}
	if(sbuff.pos>=num_of_bit){
		result= sbuff.data>>(sbuff.pos= sbuff.pos-num_of_bit);// result có giá trị bằng (num_of_bit) đầu tiên của data.	
		sbuff.data &=~(tmp<<sbuff.pos);// xóa (num_of_bit) đầu tiên của data về giá trị 0.			
	}	
	return result;
}
int import_to_des_buffer(){// hàm này thực hiện ghi một (num_of_bit) bit dữ liệu từ file vào des buffer-data.
	unsigned int result;
	if(dbuff.pos+num_of_bit<BUFFER_LEN){
		if((result=export_from_src_buffer())<MAX_VAL){
			dbuff.pos+=num_of_bit;
			dbuff.data=(dbuff.data<<num_of_bit)+result;			
			return 1;
		}
		else{			
			puts("Het sach du lieu!");
			return 0;
		}
	}
}
void export_from_des_buffer(){
	unsigned int tmp=(2<<BYTE_LEN)-1, result=MAX_VAL;
	char c;
	while(dbuff.pos<BYTE_LEN){			
		if(import_to_des_buffer()==0)	break;		
	}
	if(dbuff.pos>=BYTE_LEN){
		c= dbuff.data>> (dbuff.pos= dbuff.pos-BYTE_LEN);// c có giá trị bằng (num_of_bit) đầu tiên của data.	
		fwrite(&c,sizeof(char),1,df);		
		dbuff.data &=~(tmp<<dbuff.pos);// xóa 8 bit đầu tiên của data về giá trị 0.				
	}	
}