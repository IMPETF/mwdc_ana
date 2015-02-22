#include <stdio.h>
#include "TString.h"
#include "TSystem.h"

void init_check(const char* dir,const char* prefix,const char* suffix="at1")
{
  printf("### Bunch ID Checking ###\n");
  //
  const int fileid[9]={1,2,3,4,5,13,14,15,16};
  //
  FILE* fp[9];
  TString fname[9];
  for(int i=0;i<9;i++){
    fname[i].Form("%s/%s%d.%s",dir,prefix,fileid[i],suffix);
    fp[i]=fopen(fname[i].Data(),"rb");
    if(!fp[i]){
      printf("error opening file: %s\n",fname[i].Data());
      exit(1);
    }
  }
  //
  printf("===> fileid:\n");
  for (int i = 0 ; i <9; ++i) {
    printf("%d\t",fileid[i]);
  }
  printf("\n\n");
  //
  int chkevent=5;
  int bunch_id[9];
  int event_len;
  unsigned int trigger_id=0;
  unsigned int* buffer;
  int counter=0;
  //IMPORTANT: buffer should be large enough to hold an event,otherwise fread error may occur.
  buffer=(unsigned int*)malloc(sizeof(unsigned int)*8192);
  
  for(int i=0;i<chkevent;i++){
    gSystem->Sleep(1000);
    printf("===> Event_%d:\n",i+1);
    //
    for(int j=0;j<9;j++){
      counter=fread(&trigger_id,1,4,fp[j]);
      if(counter<4){
	if(ferror(fp[j])){
	  printf("fread error\n");
	  exit(1);
	}
	else{
	  break;
	}
      }
      //
      counter=fread(&event_len,1,4,fp[j]);
      if(counter<4){
	if(ferror(fp[j])){
	  printf("fread error\n");
	  exit(1);
	}
	else{
	  break;
	}
      }
      //
      counter=fread(buffer,1,event_len,fp[j]);
      if(counter!=event_len){
	if(ferror(fp[j])){
	  printf("unexpected behavior\n");
	  exit(1);
	}
	break;
      }
      //
      if((buffer[0]>>28)==0){
	bunch_id[j]=buffer[0]&0xFFF;
      }
      else{
	printf("not group header\n");
      }
    }
    //
    for(int j=0;j<9;j++){
      printf("%d\t",bunch_id[j]);
    }
    printf("\n\n",i+1);
  }
  //
  free(buffer);
  for(int i=0;i<9;i++){
    fclose(fp[i]);
  }
  
}


char _GetNextEvent_ungrouped(FILE*fp,unsigned int** old_buffer,int* buffer_len,int* event_len,int* bunch_id,int* event_id,int* word_count)
{
  unsigned int* new_buffer,*buffer;
  buffer=*old_buffer;
  unsigned int gheader,gtrailer;
  int type_id,eventid_trailer;
  *event_len=0;
  *word_count=-1;
  //GroupHeader
  if(fread(&gheader,4,1,fp) <1){
    if(feof(fp))
      return 0;
    else
      return -1;
  }
  else{
    type_id=gheader>>28;
    if(type_id==0x0){
      *bunch_id=gheader&0xFFF;
      *event_id=(gheader>>12)&0xFFF;
    }
    else{
      return -2;
    }
  }
  //
  while(type_id!=0x1){
    //Read next word
    if(fread(buffer+(*event_len),4,1,fp) <1){
      if(feof(fp))
	return 0;
      else
	return -1;
    }
    else{
      type_id=buffer[*event_len]>>28;
      if(type_id==0x4 || type_id==0x5 || type_id==0x6){
	(*event_len)++;
      }
      else if(type_id==0x1){
	*word_count=buffer[*event_len]&0xFFF;
	eventid_trailer=(buffer[*event_len]>>12)&0xFFF;
	continue;
      }
      else if(type_id==0x0){
	fseek(fp,-4,SEEK_CUR);
	break;
      }
      else{
	return -3;
      }
    }
    //Expand Buffer size in case of large events
    if((*event_len)==(*buffer_len)){
      new_buffer=(unsigned int*)malloc(((*buffer_len)+1024)*4);
      memcpy(new_buffer,buffer,(*buffer_len)*4);
      (*buffer_len)+=1024;
      free(buffer);
      buffer=new_buffer;
      (*old_buffer)=new_buffer;
      printf("buffer expanded(new buffer_len:%d,event_len:%d)\n",*buffer_len,*event_len);
    }
  }
  //
  if(type_id==0x0){
    *word_count=-1;
    return 2;
  }
  else if((*event_id)!=eventid_trailer){
    return 3;
  }
  else{
    return 1;
  }
}
  
void init_check_ungrouped(const char* dir,const char* prefix,const char* suffix="at1")
{
  printf("### Bunch ID Checking ###\n");
  //
  const int fileid[9]={1,2,3,4,5,13,14,15,16};
  //
  FILE* fp[9];
  TString fname[9];
  for(int i=0;i<9;i++){
    fname[i].Form("%s/%s%d.%s",dir,prefix,fileid[i],suffix);
    fp[i]=fopen(fname[i].Data(),"rb");
    if(!fp[i]){
      printf("error opening file: %s\n",fname[i].Data());
      exit(1);
    }
  }
  //
  printf("===> fileid:\n");
  for (int i = 0 ; i <9; ++i) {
    printf("%d\t",fileid[i]);
  }
  printf("\n\n");
  //
  int chkevent=100;
  int event_id[9];
  int bunch_id[9];
  int event_len,word_count;
  char event_flag=1;
  
  unsigned int* buffer;
  int buffer_len=2048;
  buffer=(unsigned int*)malloc(sizeof(unsigned int)*buffer_len);
  
  for(int i=0;i<chkevent;i++){
    gSystem->Sleep(1000);
    printf("===> Event_%d:\n",i+1);
    //
    for(int j=0;j<9;j++){
      event_flag=_GetNextEvent_ungrouped(fp[j],&buffer,&buffer_len,&event_len,bunch_id+j,event_id+j,&word_count);
      if(event_flag>0){
	printf("%d\t",bunch_id[j]);
      }
      else{
	switch(event_flag){
	  case -1:
	    printf("(%s%d)fread() error\n",prefix,fileid[j]);
	    break;
	  case -2:
	    printf("(%s%d)the first word is not group header\n",prefix,fileid[j]);
	    break;
	  case -3:
	    printf("(%s%d)unexpected type_id in packet\n",prefix,fileid[j]);
	    break;
	  default:
	    printf("(%s%d) EOF reached\n",prefix,fileid[j]);
	    break;
	}
      }
    }
    printf("\n\n",i+1);
  }
  //
  free(buffer);
  for(int i=0;i<9;i++){
    fclose(fp[i]);
  }
}