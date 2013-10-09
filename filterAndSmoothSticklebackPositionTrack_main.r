####


rm(list=ls())
source('/home/reza/Dropbox/stickleback_r_newScript/Stickleback-Tracking-R-Code/filterAndSmoothSticklebackPositionTrack_functions.r')



rawInputFile = '/home/reza/Dropbox/stickleback_r_newScript/Stickleback-Tracking-R-Code/1.4_RP-clip-2011-05-06 14;01;36_allFramesInfoSorted_raw.txt';
filteredInputFile = paste(substr(rawInputFile,1,nchar(rawInputFile)-4),"_filtered.txt",sep="");
smoothedInputfile = paste(substr(filteredInputFile,1,nchar(filteredInputFile)-4),"_smoothed.txt",sep="");

filterSticklebackTrackFile(rawInputFile);
system.time(smoothSticklebackTrackFile(filteredInputFile,4000, 5000))




