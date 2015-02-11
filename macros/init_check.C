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