####


rm(list=ls())
source('/home/reza/Dropbox/stickleback_r_newScript/Stickleback-Tracking-R-Code/filterAndSmoothSticklebackPositionTrack_functions.r')



rawInputFile = '/home/reza/Dropbox/stickleback_r_newScript/Stickleback-Tracking-R-Code/1.4_RP-clip-2011-05-06 14;01;36_allFramesInfoSorted_raw.txt';
filteredInputFile = paste(substr(rawInputFile,1,nchar(rawInputFile)-4),"_filtered.txt",sep="");
smoothedInputfile = paste(substr(filteredInputFile,1,nchar(filteredInputFile)-4),"_smoothed.txt",sep="");

filterSticklebackTrackFile(rawInputFile);

system.time(smoothSticklebackTrackFile(filteredInputFile,4000, 5000))



################################# NOW RUN THE SCRIPT OVER ALL OF THE VIDEOS ########################

rm(list=ls())
source('C:/Users/dehestan/Dropbox/stickleback_r_newScript/Stickleback-Tracking-R-Code/filterAndSmoothSticklebackPositionTrack_functions.r')


inputListOfFiles = 'C:/Users/dehestan/Dropbox/stickleback_r_newScript/data/PJxPJ_F2_2011/videoinfo1of4_fullPath_schoolOnly_changedAddress.csv';
listOfFiles = read.csv(inputListOfFiles, header = TRUE)
listOfFailedFiles = c();
listOfAlreadyExistedFiles = c();
Num = length(listOfFiles$filename);


for (i in 1:Num)
{
  print(sprintf(" %d out of %d",i,Num));
  rawInputFile = as.character(listOfFiles$filename[i]);
  
  if (!file.exists(rawInputFile))
  {
    listOfFailedFiles = c(listOfFailedFiles,rawInputFile)
    next;
  }
  

  
  filteredInputFile = paste(substr(rawInputFile,1,nchar(rawInputFile)-4),"_filtered.txt",sep="");
  smoothedInputfile = paste(substr(filteredInputFile,1,nchar(filteredInputFile)-4),"_smoothed.txt",sep="");
  #this is for the problem that I had for some of the smoothed file since there was a bug in the smoothSticklebackTrackFile function
  if(file.exists(smoothedInputfile))
  {
    
    listOfAlreadyExistedFiles = c(listOfAlreadyExistedFiles, smoothedInputfile);
    next;
    
  }
  
  firstFrame = as.numeric(listOfFiles$startframe[i]);
  lastFrame = as.numeric(listOfFiles$stopframe[i]);
  
  system.time(filterSticklebackTrackFile(rawInputFile));
  
  system.time(smoothSticklebackTrackFile(filteredInputFile,firstFrame,lastFrame));

}


listOfFailedFileName = paste(substr(inputListOfFiles,1,nchar(inputListOfFiles)-4),"_failed.txt",sep="");
write.csv(listOfFailedFiles, file = listOfFailedFileName)


listOfAlreadyExistedFilesName = paste(substr(inputListOfFiles,1,nchar(inputListOfFiles)-4),"_failed.txt",sep="");
write.csv(listOfAlreadyExistedFiles, file = listOfAlreadyExistedFilesName)
