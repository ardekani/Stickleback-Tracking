#### this file is another attempt to unify post processing of stickleback data...

# 1- Smoothing the ceter position files -- (this is in matlab, needs to be converted to R)
# 2- Smoothing the position (and direction of Stickleback)
#    a- current version simply ignores the frames that we have more than one detected components (which is not really a bad strategy!) and replace them with NA values
#    b- if in the processing time, the __KEEP_JUST_THE_LARGEST flag is set to 1 in c++ prog this step is not neccessary because we anyhow will have just one component
#    c- what mentioned is b, is not the best strategy, we should be keeping all components and then try to find out which one is more likely to be real fish based on the tracking and position of fish in previous frame



################### Lets ignore all frames that have more than one measurement! this part is not needed in the case of b- see above##########################
# # below code is not finished ...
# rm(list=ls())
# #setwd("C:/Users/dehest01/Dropbox/RCode_StickleBack")
# #rawData = read.csv("schooling1_infoAfter9000Removed.txt", fill = TRUE, header = F, sep=",")
# inFileName = '';
# outFileName = '';
# setwd("E:/myDropBox/Dropbox/RCode_StickleBack");
# rawData = read.csv("schooling1_info-justlargest.txt")
# 
# firstFrame = 30;
# lastFrame = 9030;
# rawData = rawData[which(rawData[,1]==firstFrame):which(rawData[,1]==lastFrame),];
# 
# 
# NcThresh = 1;
# Nc = rawData[,2]; 
# badFrames = which(Nc>NcThresh);
# newData = rawData;
# newData[badFrames,3:ncol(rawData)]=rep(NA,1,ncol(rawData)-2);# rawData[badFrames,-(2:ncol(rawData))];
# newData = newData[,-c((2+NcThresh*4+1):ncol(rawData))]
# names(newData) <- c("fn","NC","xdir","ydir","x","y")
# 
# plot(abs(diff(newData$x)))
# 
# newData = newData[-which(is.na(newData$xdir)),]
# 
# write.table(newData, file = "original2.csv",sep=",",row.names=FALSE, col.names = TRUE)
# ##########################################################################################################
# 
# #run from here if the KEEP_LARGEST_FLAG has been set 1 in c++ code - this is the case for ICPR Workshop paper
# # make sure the input file has 5 comma(seperator in the first line)- most of the time this is not the case
# # TODO: modify the c++ code to make sure this happens...
# rm(list=ls())
# firstFrame = 30;
# #lastFrame = 9030;
# lastFrame = 9999;
# #inFileName = "schooling1_info_9030_largest.txt"
# inFileName = "E:/myDropBox/Dropbox/RCode_StickleBack/schoolingData/schooling3_ws10_mp352_WithFeat_fromFile.txt";
# inFileName = "E:/myDropBox/Dropbox/RCode_StickleBack/schooling1_info_9030_largest.txt";
# inFileName = "C:/Users/dehestan/Dropbox/stickleback_EUROSIP/videos/PJxP_Family1/school_only/1.1_GG-clip-2011-04-21 16;05;32_ws16_mp350_WithFeat_fromVideo.txt";
# inFileName = "C:/Users/dehestan/Dropbox/stickleback_EUROSIP/annotated videos/school-only/2.3_BB-clip-2011-06-02 17;28;29/2.3_BB-clip-2011-06-02 17;28;29_ws16_mp350_WithFeat_fromVideo_filtered.txt";
####################
source('/home/reza/Dropbox/stickleback_r_newScript/Stickleback-Tracking-R-Code/filterAndSmoothSticklebackPositionTrack_functions.r')








