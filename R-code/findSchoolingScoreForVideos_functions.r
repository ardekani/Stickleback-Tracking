#########################################################################
#the assumption for this function is that in the input vector, 0 means not schooling, 1 means schooling and 2 means NA
fillGapsBetweenSchooling <- function(isSchooling,maxGapSize = 10)
{
  
  ############ NOW FIX INBETWEEN POINTS###############
  ## THE INTERVALS THAT ARE NA OR 2 , IF THEY ARE BETWEEN 1'S , ALL 2 SHOULD BE CONVERTED TO 1'S AS
  # FISH MUST BE SCHOOLING AND IT GOT LOST IN THE DETECTION PART
  dfSchool = diff(isSchooling)
  ## so the area between any 1 and -1 in the diff
  
  dfSchool = c(dfSchool,0)
  
  stack1 = which(dfSchool==1)
  stack2 = which(dfSchool == -1)
  
  ###############################
  plus1 = c();
  minus1 = c();
  
  while(length(stack1)>0 & length(stack2)>0)
  {
    while( stack1[1]>stack2[1] )
    {
      stack2 = stack2[-1];
      if (length(stack2)<1)
        break;
    }
    
    if (length(stack2)<1)
      break;
    

    if (sum(dfSchool[(stack1[1]+1):(stack2[1]-1)])==0)  {
      
      plus1  = c(plus1 ,stack1[1]+1)
      minus1 = c(minus1,stack2[1])
      
      stack1 = stack1[-1]
      stack2 = stack2[-1]
    }
    else
    {
      stack1 = stack1[-1]
    }
  }

  filledGapsSchooling = isSchooling;
  if ( ( length(plus1)==length(minus1) ) & (length(plus1)>0) )
  {
    for (ll in 1:length(plus1))
    {
      if(abs(plus1[ll]-minus1[ll])<maxGapSize)
      filledGapsSchooling[plus1[ll]:minus1[ll]] = 1;
    }
  }
  
  return(filledGapsSchooling)
}
###################################################################
#############################################################################

calculateSchoolingScoreForThisVideo <- function(fileName, firstFrame=1, lastFrame=9000, threshForCloseness=125, threshForTimeOfCloseNessInEvery30 = 15)
#checkThisVideo2 <- function(fileName, firstFrame=1, lastFrame=9000, threshForCloseness=125, threshForVel = 1.5)
{
  # these are what have been used in the current version

  #threshForCloseness = 125;
  #threshForVel = 1.5; 
  
  #firstFrame = 0;
  #lastFrame = 9000;
  #fileName = "C:\\stickleback\\tuningParameters\\videos\\F2.1_GR-clip-2011-10-25 12;32;47.avi"
  
  if (firstFrame==0)
  {
    firstFrame = 1;
    lastFrame = lastFrame+1;
  }
  

  
  centerFileName  = paste(substr(fileName,0,nchar(fileName)-4),"_centerOfCircle_smooth.txt",sep="");
  fishPosFileName = paste(substr(fileName,0,nchar(fileName)-4),"_allFramesInfoSorted_raw_filtered_smoothed.txt",sep="");
  fishPosBeforeSmoothingFileName = paste(substr(fileName,0,nchar(fileName)-4),"_allFramesInfoSorted_raw_filtered.txt",sep="");
  outputFileName  = paste(substr(fileName,0,nchar(fileName)-4),"_schoolingScore_per_second.csv",sep="");
  
  if(!file.exists(centerFileName) || !file.exists(fishPosFileName) || !file.exists(fishPosBeforeSmoothingFileName))
  {
    
    toRet = data.frame(videoName = as.character(fileName),schoolingScore = -1, confidence = -1 )
    return(toRet);
    
  }
  
  
  fishPosBeforeSmoothing = read.csv(fishPosBeforeSmoothingFileName, header=FALSE);
  #print(dim(fishPosBeforeSmoothing)[1])
  if(dim(fishPosBeforeSmoothing)[1]<4500) # if we do not detect the fish in at least half of the frames.. there is something wrong with that video, we don't score it
  {
    
    toRet = data.frame(videoName = as.character(fileName),schoolingScore = -1, confidence = -1 )
    return(toRet);
    
  }
  

  rawCenter = read.csv(centerFileName, header=FALSE);
  rawFish = read.csv(fishPosFileName, header=FALSE);
  
  if(rawCenter[1,1]==0) #simply remove the first frame with 0 frame number!
  {
    
    rawCenter = rawCenter[-1,];
  }
  
  if(rawFish[1,1]==0) #simply remove the first frame with 0 frame number!
  {
    rawFish = rawFish[-1,];
  }
  

  
  rawCenter_frames = rawCenter[,1];

  rawFish_frames   = rawFish[,1];
  
  if(firstFrame>1)# to avoid having -1 frame number for the videoes that first frame is 0 -- see the first 'if' on top of this function
  {
    rawFish_frames   = rawFish[,1] - firstFrame; # it seems there is an error on the output of the videos, frame numbers are 1 ..n + 2*startframe!
  }
  
  fishX       = rep(NA,lastFrame);
  fishY       = rep(NA,lastFrame);
  rawCenterX  = rep(NA,lastFrame);
  rawCenterY  = rep(NA,lastFrame);
  
  fishX[rawFish_frames] = rawFish[,4];
  fishY[rawFish_frames] = rawFish[,5];
  rawCenterX[rawCenter_frames]  = rawCenter[,2];
  rawCenterY[rawCenter_frames]  = rawCenter[,3];
  
  fishX = fishX[(firstFrame+1):lastFrame];
  fishY = fishY[(firstFrame+1):lastFrame];
  rawCenterX = rawCenterX[(firstFrame+1):lastFrame];
  rawCenterY = rawCenterY[(firstFrame+1):lastFrame];
  
  
  fishPos = cbind(fishX,fishY);
  modelPos = cbind(rawCenterX,rawCenterY);
  distance = (fishPos-modelPos)^2; 
  distance = sqrt(rowSums(distance));
  
  isClose=rep(0,length(distance))
  isClose[distance<threshForCloseness] = 1;
  isClose[is.na(distance)] = NA;
  sum(is.na(isClose));
  
  isCloseMat = matrix(isClose,30,300)
  
  isCloseSec = colSums(isCloseMat)
  
  isCloseSec[isCloseSec<threshForTimeOfCloseNessInEvery30] = 0;
  isCloseSec[isCloseSec>threshForTimeOfCloseNessInEvery30] = 1;
  
  
  #isCloseSec[isCloseSec<15]  = 0;
  #isCloseSec[isCloseSec>=15] = 1;
  
  # now we wanna say they should atleast be close 2 or more seconds
  isSchooling = matrix(isCloseSec,2,150); # do a trick, find the 
  isSchooling = colSums(isSchooling)
  
  isSchooling[isSchooling<2]  = 0;
  isSchooling[isSchooling>=2] = 1;
  
  isSchooling = rbind(isSchooling, isSchooling);
  
  isSchooling = as.numeric(matrix(isSchooling,1,300));
  
  confidence = 300-sum(is.na(isSchooling));
  confidence = (100*confidence)/300;
  
  outputInfo = data.frame(isSchooling);
  
  write.csv(outputInfo,file=outputFileName,row.names = FALSE, col.names = FALSE)
  
  toRet = data.frame(videoName = as.character(fileName), schoolingScore = sum(isSchooling, na.rm = TRUE), confidence = confidence);
    
  return(toRet);
}


####################### THIS IS A NEW IMPLEMENTATION THAT USES NON - SMOOTHED TRACK FILE FOR THE FISH POSITION

# 

calculateSchoolingScoreForThisVideo2 <- function(fileName, firstFrame=1, lastFrame=9000, threshForCloseness=135, threshForTimeOfCloseNessInEvery30 = 15, threshForSpeed = 20, maxGapSizeTofill=10)
{
  
  if (firstFrame==0)
  {
    firstFrame = 1;
    lastFrame = lastFrame+1;
  }
  
  
  centerFileName  = paste(substr(fileName,0,nchar(fileName)-4),"_centerOfCircle_smooth.txt",sep="");
  fishPosFileName = paste(substr(fileName,0,nchar(fileName)-4),"_allFramesInfoSorted_raw_filtered.txt",sep="");
  outputFileName  = paste(substr(fileName,0,nchar(fileName)-4),"_schoolingScore_per_second_new.csv",sep="");
  
  if(!file.exists(centerFileName) || !file.exists(fishPosFileName) )
  {
    
    toRet = data.frame(videoName = as.character(fileName),schoolingScore = -1, confidence = -1 )
    return(toRet);
    
  }


  rawCenter = read.csv(centerFileName, header=FALSE);
  rawFish = read.csv(fishPosFileName, header=FALSE);

  if(dim(rawFish)[1]<4500) # if we do not detect the fish in at least half of the frames.. there is something wrong with that video, we don't score it
  {
    
    toRet = data.frame(videoName = as.character(fileName),schoolingScore = -1, confidence = -1 )
    return(toRet);
    
  }
  
  
  if(rawCenter[1,1]==0) #simply remove the first frame with 0 frame number!
  {
    
    rawCenter = rawCenter[-1,];
  }

  if(rawFish[1,1]==0) #simply remove the first frame with 0 frame number!
  {
    rawFish = rawFish[-1,];
  }


  rawCenter_frames = rawCenter[,1];
  
  rawFish_frames   = rawFish[,1];

  if(firstFrame>1)# to avoid having -1 frame number for the videoes that first frame is 0 -- see the first 'if' on top of this function
  {
    rawFish_frames   = rawFish[,1] - firstFrame; # it seems there is an error on the output of the videos, frame numbers are 1 ..n + 2*startframe!
  }

############# remove number of detected components###########
  rawFish = rawFish[,-2];
################calculating distance betwween fish and model #######################

  fishX       = rep(NA,lastFrame);
  fishY       = rep(NA,lastFrame);
  rawCenterX  = rep(NA,lastFrame);
  rawCenterY  = rep(NA,lastFrame);
  
  fishX[rawFish_frames] = rawFish[,4];
  fishY[rawFish_frames] = rawFish[,5];
  rawCenterX[rawCenter_frames]  = rawCenter[,2];
  rawCenterY[rawCenter_frames]  = rawCenter[,3];
  
  fishX = fishX[(firstFrame+1):lastFrame];
  fishY = fishY[(firstFrame+1):lastFrame];
  rawCenterX = rawCenterX[(firstFrame+1):lastFrame];
  rawCenterY = rawCenterY[(firstFrame+1):lastFrame];
  
  
  fishPos = cbind(fishX,fishY);
  modelPos = cbind(rawCenterX,rawCenterY);
  distance = (fishPos-modelPos)^2; 
  distance = sqrt(rowSums(distance));


#################calculating the speed of the fish (pixel/second)###################
  fishXMat = matrix(fishX,30,300)
  fishXMedian = apply(fishXMat, 2, median,na.rm=TRUE)
  
  fishYMat = matrix(fishY,30,300)
  fishYMedian = apply(fishYMat, 2, median,na.rm=TRUE)
  
  Vf = sqrt(diff(fishXMedian)^2 + diff(fishYMedian)^2);
  Vf = c(Vf,Vf[length(Vf)]);
  
  
  VVf = sqrt(diff(fishX)^2 + diff(fishY)^2);
  VVf = c(VVf,VVf[length(VVf)]);
  VVfMat = matrix(VVf,30,300);
  VVfEst = apply(VVfMat, 2, median,na.rm=TRUE)
  
  
  FishIsNotMovingSeconds = which(Vf<threshForSpeed)

  #now finding the angle of movement
  fAngle = 360*atan2(diff(fishYMedian),diff(fishXMedian))/(2*pi)
  
  
  
  #
  
  modelXMat = matrix(rawCenterX,30,300)
  modelXMedian = apply(modelXMat, 2, median,na.rm=TRUE)
  
  modelYMat = matrix(rawCenterY,30,300)
  modelYMedian = apply(modelYMat, 2, median,na.rm=TRUE)
  
  
  Vm = sqrt(diff(modelXMedian)^2 + diff(modelYMedian)^2);
  Vm = c(Vm,Vm[length(Vm)]);
  
  
  mAngle = 360*atan2(diff(modelYMedian),diff(modelXMedian))/(2*pi);
  
  
  #print(fAngle)
  #print(mAngle)
  
  dfAngle = abs(mAngle-fAngle);
  dfAngle[which(dfAngle>180)] = 360-dfAngle[which(dfAngle>180)];
  
  dfAngle = c(dfAngle,0)
  notSameDirection = (dfAngle > 90);
  
  fishIsNotInTheSameDirectionIndx = which(notSameDirection==TRUE)
  
##################################

  isClose=rep(0,length(distance))
  isClose[distance<threshForCloseness] = 1;
  isClose[is.na(distance)] = NA;
  sum(is.na(isClose));
  
  isCloseMat = matrix(isClose,30,300)
  
  distanceMat = matrix(distance,30,300)
  distanceSec = colSums(distanceMat,na.rm = TRUE)/30;

  NAinCloseNess = colSums(is.na(isCloseMat));                 #number of NA in each column (second) in closeness 
  ZeroinCloseNess = colSums(isCloseMat==0, na.rm = TRUE);     #number of zeros in each column(second) in closeness 
  OneinCloseNess  = colSums(isCloseMat==1, na.rm = TRUE);     #number of zeros in each column(second) in closeness

  #NAinCloseNess[NAinCloseNess<20] = 0; # this was what is was that I chated with  Anna about it
  NAinCloseNess[NAinCloseNess<10] = 0; 
  
  tags = cbind(ZeroinCloseNess,OneinCloseNess,NAinCloseNess); # puting all together column wise to fin

  mx = max.col(tags); # max = 1 means most of the frames in this second fish and model were not close, 
                      # max = 2 means most of the frames in this second fish and model were close
                      # max = 3 means most of the frames in this second fish and model were NA

  isSchooling = mx-1; # 0 = not schooling, 1:schooling, 2: NA
  beforeSpeedCorrection = isSchooling;
  
  
  #isSchooling[FishIsNotMovingSeconds] = 0; #if speed is less than a threshold, there is no schooling!
  print(FishIsNotMovingSeconds)

#   print("fishIsNotInTheSameDirectionIndx")
#   print(fishIsNotInTheSameDirectionIndx)
#   counter = 0;
#   for (ii in length(fishIsNotInTheSameDirectionIndx))
#   {
#     if (isSchooling[fishIsNotInTheSameDirectionIndx[ii]]==1)
#     {
#       isSchooling[fishIsNotInTheSameDirectionIndx[ii]] = 0;
#       counter = counter + 1;
#       print(" --- here ---")
#       print(fishIsNotInTheSameDirectionIndx[ii]);
#     }
#   }
#   
#     print("after ApplyingSameDirection")
#     print(counter)

  
  
#print(isSchooling)
  
  isSchooling[FishIsNotMovingSeconds] = 0; #if speed is less than a threshold, there is no schooling!
  
  isSchooling = fillGapsBetweenSchooling(isSchooling,maxGapSizeTofill);

  

  
#print(isSchooling)
    
  isSchooling = as.numeric(matrix(isSchooling,1,300));
  
  confidence = 300-sum(isSchooling==2);
  confidence = (100*confidence)/300;
  
  outputInfo = data.frame(isSchooling,Vf,dfAngle,tags,distanceSec );
  
  write.csv(outputInfo,file=outputFileName,row.names = FALSE, col.names = FALSE)
  
  toRet = data.frame(videoName = as.character(fileName), schoolingScore = sum(isSchooling==1), confidence = confidence);
  
  return(toRet);
  

}









