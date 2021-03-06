rm(list=ls())
setwd("C:/Users/dehestan/Dropbox/stickleback_r_newScript/Stickleback-Tracking-R-Code/")
source ('findSchoolingScoreForVideos_functions.r');
 #threshForSpeed   = 20;
 #maxGapSizeTofill =15;


timeofCloseNessThresholds = 15;
distanceThresholds = 135;
maxGapForSchooling = 3;
threshSpeed = 20;



#inputListOfFiles = 'C:/Users/dehestan/Dropbox/stickleback_r_newScript/data/PJxPJ_F2_2011/videoinfo1of4_fullPath_schoolOnly_changedAddress_toCalculateSchoolingScore.csv';
#inputListOfFiles = 'C:/Users/dehestan/Dropbox/stickleback_r_newScript/data/PJxP_Family1/videoinfo4of4_fullPath_school_only_for_calculating_the_schooling.csv'
#inputListOfFiles = 'C:/Users/dehestan/Dropbox/stickleback_r_newScript/data/PJxP_Family4/videoinfo3of4_fullPath_schooling_only_address_forCalculatingSchoolingScore.csv'
inputListOfFiles = 'C:/Users/dehestan/Dropbox/stickleback_r_newScript/data/PxPJ_Family2/videoinfo2of4_fullPath_school_only_forSchoolingScore.csv'

#C:\Users\dehestan\Dropbox\stickleback_r_newScript\data\PxPJ_Family2\videoinfo2of4_fullPath_school_only_forSchoolingScore.csv

#inputListOfFiles = 'C:/Users/dehestan/Dropbox/stickleback_r_newScript/data/PJxP_Family4/videoinfo3of4_fullPath_schooling_only_address_forCalculatingSchoolingScore_just9videos.csv';
#inputListOfFiles = "C:/stickleback/tuningParameters/trainingVideos_full_videos.csv";

listOfFiles = read.csv(inputListOfFiles, header = TRUE)

listOfFailedFiles = c();
Num = length(listOfFiles$filename);

wholeResult = c();
for (i in 1:Num)
#for (i in 1:1)  
{
  print(sprintf(" %d out of %d",i,Num));
  rawInputFile = as.character(listOfFiles$filename[i]);
  
  #rawInputFile = "C:/Users/dehestan/Dropbox/stickleback_r_newScript/data/PxPJ_Family2/school_only/2.7_OG-clip-2011-06-07 13;26;22.mp4"
  #firstFrame = 29;
  #lastFrame = 9029;
#   if (!file.exists(rawInputFile))
#   {
#     c(listOfFailedFiles,rawInputFile)
#     next;
#   }
#   
  
  firstFrame = as.numeric(listOfFiles$startframe[i]);
  lastFrame = as.numeric(listOfFiles$stopframe[i]);

  # 135 and 15 are selected thresholds... they are good!
  ret = calculateSchoolingScoreForThisVideo2(rawInputFile, firstFrame, lastFrame,distanceThresholds,timeofCloseNessThresholds,threshSpeed, maxGapForSchooling)
  
  
  #oldMethod
  #ret = calculateSchoolingScoreForThisVideo(rawInputFile, firstFrame, lastFrame,135,15 )
  
  
  wholeResult = rbind(wholeResult,ret)
  
}


wholeResult = data.frame(wholeResult);
names(wholeResult) = c("videoName","schoolingScore","reliability");
wholeResultOutputFileName = paste(substr(inputListOfFiles,0,nchar(inputListOfFiles)-4),"whole_results_Jan2014.csv")
write.csv(wholeResult,file = wholeResultOutputFileName, row.names=FALSE, quote = FALSE);

sum(wholeResult$schoolingScore == -1)
print(wholeResult)














dt1 = read.csv('C:/stickleback/tuningParameters/trainingVideos_full_videos whole_results_new_135_15.csv');

auto = dt1$schoolingScore

dt = read.csv('C:/stickleback/tuningParameters/trainingVideos_full_videos whole_results_new_135_15_toCompareNewMethod.csv')

manual = dt$manual;

auto[which(manual<20)]


dt2 = read.csv('C:/stickleback/tuningParameters/trainingVideos_full_videos whole_results_old_135_15.csv');
autoOld = dt2$schoolingScore

autoOld[which(manual<20)] - manual[which(manual<20)]

auto[which(manual<20)] - manual[which(manual<20)]


