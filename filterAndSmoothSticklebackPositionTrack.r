# this file is another attempt to unify post processing of stickleback data...

# 1- Smoothing the ceter position files -- (this is in matlab, needs to be converted to R)
# 2- Smoothing the position (and direction of Stickleback)
#    a- current version simply ignores the frames that we have more than one detected components (which is not really a bad strategy!) and replace them with NA values
#    b- if in the processing time, the __KEEP_JUST_THE_LARGEST flag is set to 1 in c++ prog this step is not neccessary because we anyhow will have just one component
#    c- what mentioned is b, is not the best strategy, we should be keeping all components and then try to find out which one is more likely to be real fish based on the tracking and position of fish in previous frame



################### Lets ignore all frames that have more than one measurement! this part is not needed in the case of b- see above##########################
# below code is not finished ...
rm(list=ls())
#setwd("C:/Users/dehest01/Dropbox/RCode_StickleBack")
#rawData = read.csv("schooling1_infoAfter9000Removed.txt", fill = TRUE, header = F, sep=",")
inFileName = '';
outFileName = '';
setwd("E:/myDropBox/Dropbox/RCode_StickleBack");
rawData = read.csv("schooling1_info-justlargest.txt")

firstFrame = 30;
lastFrame = 9030;
rawData = rawData[which(rawData[,1]==firstFrame):which(rawData[,1]==lastFrame),];


NcThresh = 1;
Nc = rawData[,2]; 
badFrames = which(Nc>NcThresh);
newData = rawData;
newData[badFrames,3:ncol(rawData)]=rep(NA,1,ncol(rawData)-2);# rawData[badFrames,-(2:ncol(rawData))];
newData = newData[,-c((2+NcThresh*4+1):ncol(rawData))]
names(newData) <- c("fn","NC","xdir","ydir","x","y")

plot(abs(diff(newData$x)))

newData = newData[-which(is.na(newData$xdir)),]

write.table(newData, file = "original2.csv",sep=",",row.names=FALSE, col.names = TRUE)
##########################################################################################################

#run from here if the KEEP_LARGEST_FLAG has been set 1 in c++ code - this is the case for ICPR Workshop paper
# make sure the input file has 5 comma(seperator in the first line)- most of the time this is not the case
# TODO: modify the c++ code to make sure this happens...
rm(list=ls())
firstFrame = 30;
#lastFrame = 9030;
lastFrame = 9999;
#inFileName = "schooling1_info_9030_largest.txt"
inFileName = "E:/myDropBox/Dropbox/RCode_StickleBack/schoolingData/schooling3_ws10_mp352_WithFeat_fromFile.txt";
inFileName = "E:/myDropBox/Dropbox/RCode_StickleBack/schooling1_info_9030_largest.txt";
inFileName = "C:/Users/dehestan/Dropbox/stickleback_EUROSIP/videos/PJxP_Family1/school_only/1.1_GG-clip-2011-04-21 16;05;32_ws16_mp350_WithFeat_fromVideo.txt";
inFileName = "C:/Users/dehestan/Dropbox/stickleback_EUROSIP/annotated videos/school-only/2.3_BB-clip-2011-06-02 17;28;29/2.3_BB-clip-2011-06-02 17;28;29_ws16_mp350_WithFeat_fromVideo_filtered.txt";




filterEachLine <-function(line)
{
  line=as.matrix(line);
  # first check the number of the detected components
  max_Accpetable = 3
  if ((line[2] > max_Accpetable) || (is.na(line[2])))
  {
    res = line[1:(2 + max_Accpetable*];
    res[2] = 0;
  }
  else
  {
    res = line;
  }
  return(res);
}


fixThisLineWithMultipleMeasurements  <-function(data,index)
{
  
  origline = as.numeric(data[index,]);
    
  numOfDetectedParts = origline[2];#(sum(!is.na(line)) - 2)/6;
  lineMat = origline[3:(length(origline)-1)];
  dim(lineMat) = c(6, (length(lineMat))/6)
  xys=t(lineMat[3:4,]); # these are detected positions in the current frame
  xys = xys[complete.cases(xys),];
  # now we collect all of the neighbor frames that have only 1 detected objects, use their average as the true point and pickup the correct measuremnet
  
  currentFrameNum = origline[1];
  winSize = 30;
  frameDiffThresh = 30;
  topIndx = index - winSize;
  if (topIndx<1)
    topIndx =1;
  
  botIndx = index + winSize;
  if(botIndx>max(dim(data)))
    botIndx = max(dim(data));
  
  searchWindow = data[topIndx:botIndx,];
  nc = searchWindow[,2];

  
  searchWindow = searchWindow[nc ==1,];
  if(prod(dim(searchWindow))==0)
  {
      return(origline); # not fixable return original
  }
  
  fnDiff = abs(searchWindow[,1] - currentFrameNum);
  searchWindow = searchWindow[fnDiff<frameDiffThresh,]
  
  if(prod(dim(searchWindow))==0)
  {
    return(origline); # not fixable return original
  }
  
  positions = searchWindow[,5:6];
  avgXY = colMeans(positions, na.rm=TRUE);
    repAvg = rep(avgXY,numOfDetectedParts);
  dim(repAvg) = c(2,numOfDetectedParts);
  repAvg = t(repAvg);
  
  distance = apply(abs(xys-repAvg),1,sum);
  closestIndx = which(distance==min(distance))
  
  line = lineMat[,closestIndx];
    line = c(currentFrameNum,1,line,rep(NA,length(origline)-8));
  
  return(line);
  
}  
  
#remove NA's
filteredTrackData = origTrackData[complete.cases(origTrackData[,2]),];
max_Accpetable = 2;
#remove frames with more than 2 measurement
filteredTrackData = filteredTrackData[(filteredTrackData[,2]<=max_Accpetable),];

#now we want to fix the frames that have two or more than two measurements, we want to pick the right one and then 

morethanOneDetectedIndex = which(filteredTrackData[,2]>1);

for (i in 1:length(morethanOneDetectedIndex))
{
  
  fixedLine = fixThisLineWithMultipleMeasurements(filteredTrackData,index);
  filteredTrackData[index,] = fixedLine;
  
}
filteredTrackData = filteredTrackData[(filteredTrackData[,2]!=0),];

#now second round 
morethanOneDetectedIndex = which(filteredTrackData[,2]>1);

for (i in 1:length(morethanOneDetectedIndex))
{
  fixedLine = fixThisLineWithMultipleMeasurements(filteredTrackData,index);
  filteredTrackData[index,] = fixedLine;
  
}
# now the final filtereation, remove the frames that are not fixable (after two rounds)
filteredTrackData = filteredTrackData[(filteredTrackData[,2]<2),]; # keep only one measurement frames


inFileName = "/home/reza/Dropbox/stickleback_r_newScript/Stickleback-Tracking-R-Code/1.4_RP-clip-2011-05-06 14;01;36_allFramesInfoSorted_raw.txt";
n_col = count.fields(inFileName,sep=",");

origTrackData = read.csv(inFileName,header=FALSE, fill = TRUE , col.names=1:max(n_col));




outFileName=paste(substr(inFileName,1,nchar(inFileName)-4),"_smoothed.txt",sep="");

nDim = 4;
origTrackFile = read.csv(inFileName,header=FALSE)
names(origTrackFile) = c("fn","NC","xdir","ydir","x","y","useless1","useless2","useless3");

framesWithZeroComponents = which(origTrackFile$NC==0);
origTrackFile = origTrackFile[-framesWithZeroComponents,];


totalNumOfEntries = length(origTrackFile$fn)
prev_pos = as.numeric(origTrackFile[1,3:(nDim+2)]);
prev_fn = origTrackFile$fn[1];
smoothedData = c();
smoothedData = rbind(smoothedData,c(prev_fn,prev_pos));

for (ff in 2:(totalNumOfEntries-1)) 
{
  #    curr_pos = c(origTrackFile$x[ff],origTrackFile$y[ff],origTrackFile$z[ff]);
  curr_pos = as.numeric(origTrackFile[ff,3:(nDim+2)]);
  curr_fn = origTrackFile$fn[ff];
  if (curr_fn - prev_fn -1 > 0)# && (curr_fn - prev_fn -1 <10)
  {
    xdxd = round(seq(prev_pos[1],curr_pos[1], length.out = curr_fn - prev_fn -1),digits=2);
    ydyd = round(seq(prev_pos[2],curr_pos[2], length.out = curr_fn - prev_fn -1),digits=2);
    xx   = round(seq(prev_pos[3],curr_pos[3], length.out = curr_fn - prev_fn -1),digits=2);
    yy   = round(seq(prev_pos[4],curr_pos[4], length.out = curr_fn - prev_fn -1),digits=2);
    fnfn = (prev_fn+1):(curr_fn-1);
    tmp = cbind(fnfn,xdxd,ydyd,xx,yy);
    print("dim tmp = ")
    print(dim(tmp))
    print("dim smoothed data = ")
    print(dim(smoothedData))
    smoothedData = rbind(smoothedData,tmp);
    smoothedData = rbind(smoothedData,c(curr_fn,curr_pos));
    print(prev_fn);
    
  } else {
    
    smoothedData = rbind(smoothedData,c(curr_fn,curr_pos));
  }
  
  prev_pos = curr_pos;      
  prev_fn = curr_fn;
}

write.table(smoothedData,file = outFileName,sep = " ", row.names = FALSE, col.names = FALSE);


##############################even the above part is not used in ICPR paper tho.. se statisticsforstickleback2.R

