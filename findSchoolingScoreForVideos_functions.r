calculateSchoolingScoreForThisVideo <- function(fileName, firstFrame=1, lastFrame=9000, threshForCloseness=125, threshForVel = 1.5)
#checkThisVideo2 <- function(fileName, firstFrame=1, lastFrame=9000, threshForCloseness=125, threshForVel = 1.5)
{
  
  # these are what have been used in the current version
  #threshForCloseness = 125;
  #threshForVel = 1.5; 
  if (firstFrame==0)
  {
    firstFrame = 1;
    lastFrame = lastFrame+1;
  }
  #firstFrame = 1;
  #lastFrame = 9000;
  #firstFrame = 1025;
  #lastFrame = 10025;
  
  #fileName = "C:/Users/dehestan/Dropbox/stickleback_r_newScript/data/PJxP_Family1/school_only/1.1_GG-clip-2011-04-21 16;05;32.mp4"
  centerFileName  = paste(substr(fileName,0,nchar(fileName)-4),"_centerOfCircle_smooth.txt",sep="");
  fishPosFileName = paste(substr(fileName,0,nchar(fileName)-4),"_allFramesInfoSorted_raw_filtered_smoothed.txt",sep="");
  outputFileName  = paste(substr(fileName,0,nchar(fileName)-4),"_schoolingScore_per_second.csv",sep="");
  
  if(!file.exists(centerFileName) || !file.exists(fishPosFileName))
  {
    
    toRet = data.frame(videoName = as.character(fileName),schoolingScore = -1, confidence = -1 )
    return(toRet);
    
  }
  
  
  #1.1_GG-clip-2011-04-21 16;05;32_allFramesInfoSorted_raw_filtered_smoothed.txt
  #1.1_GG-clip-2011-04-21 16;05;32_centerOfCircle_smooth.txt
  
  
  #centerFileName = paste("./",fileName,"/",fileName,"_centerOfCircle_smooth.txt",sep="");
  #fishPosFileName = paste("./",fileName,"/",fileName,"_ws16_mp350_WithFeat_fromVideo_filtered.txt",sep="")
  #outputFileName = paste("./",fileName,"/",fileName,"_ws16_mp350_WithFeat_fromVideo_filtered_TAGGED.csv",sep="")
  #outputFileNameStartStops=paste("./",fileName,"/",fileName,"_ws16_mp350_WithFeat_fromVideo_filtered_StartStops.csv",sep="")

  
  
  rawCenter = read.csv(centerFileName, header=FALSE);
  rawFish = read.csv(fishPosFileName, header=FALSE);
  
  
  
  rawCenter_frames = rawCenter[,1];
  rawFish_frames   = rawFish[,1] - firstFrame; # it seems there is an error on the output of the videos, frame numbers are 1 ..n + 2*startframe!
  
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
  isCloseSec[isCloseSec<15]  = 0;
  isCloseSec[isCloseSec>=15] = 1;
  
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


  
#   plot(isSchooling)
#   
#   plot(distance[1000:2090])
#   
#   
#   
#   rawCenter = as.matrix(rawCenter[firstFrame:lastFrame,]);
#   rawFish = as.matrix(rawFish[firstFrame:lastFrame,]);
#   
# 
#   
#   
#   
#   
#   #fishPos = rawFish[,4:5];
#   fishVel = diff(fishPos);
#   
#   fishVelAbs = rowSums(fishVel^2);
#   plot(fishVelAbs);
#   
#   toRemove = which(rawFish[,2]==0);
#   
#   fishVelAbs[toRemove]=0;
#   
#   
#   fishPos = data.frame(fishPos);
#   colnames(fishPos) <- c("x","y");
#   
#   modelPos = data.frame(rawCenter[,2:3]);
#   colnames(modelPos) <- c("x","y");
#   
#   
#   #fishPos$x = averageIt2(fishPos$x)
#  # tst = averageIt2(fishPos$x)
#   #plot(seq(1/30,300,1/30),fishPos$x);
#   #lines(tst,col='blue')
#   fishPosSec = c();
#   modelPosSec = c();
#   fishPosSec$x = as.matrix(averageIt2(fishPos$x));
#   fishPosSec$y = as.matrix(averageIt2(fishPos$y));
#   
#   modelPosSec$x = as.matrix(averageIt2(modelPos$x))
#   modelPosSec$y = as.matrix(averageIt2(modelPos$y));
#   
#   ###################################################### I FIRST FIND SCHOOLING FOR EACH FRAME AND THEN DO THE 
#   fishPosSec$x = fishPos$x;
#   fishPosSec$y = fishPos$y;
#   
#   modelPosSec$x = modelPos$x;
#   modelPosSec$y = modelPos$y;
#   
#   
#   
#   # first see if fish is moving enough....
#   fishVelSec = c(0,sqrt(diff(fishPosSec$x)^2 + diff(fishPosSec$y)^2));
#   fishVelSec[which(fishVelSec>30)] = 0; # all velocities larger than 30 are treated as zero (or no-info)
#   isFishMoving=rep(0,length(fishVelSec));
#   
#   isFishMoving[fishVelSec>threshForVel] = 1;
#   plot(isFishMoving[1:1000]);
#   # now see if fish is close enough
#   
#   dist = sqrt( (fishPosSec$x-modelPosSec$x)^2 + (fishPosSec$y-modelPosSec$y)^2);
#   isClose=rep(0,length(dist))
#   isClose[dist<threshForCloseness] = 1;
#   
# #   anglef = rep(length(fishPosSec$x),0);
# #   goodf = which(fishPosSec$x>0);
# #   anglef[goodf] = (180*atan2(fishPosSec$y[goodf],fishPosSec$x[goodf]))/pi;
# #   
# #   anglem = rep(length(modelPosSec$x),0);
# #   goodm = which(modelPosSec$x>0);
# #   anglem[goodm] = (180*atan2(modelPosSec$y[goodm],modelPosSec$x[goodm]))/pi;
# #   
# #   angleDiff = abs(anglef-anglem);
# #   sameDIR = rep(length(angleDiff),0);
# #   sameDIR[which(angleDiff<10)] = 1;
# #   
# #   plot(isClose);
# #   isSchooling = (isClose & isFishMoving) & sameDIR;
# #   
# #   #isSchoolingN = filter(as.numeric(isSchooling),rep(1,10),'convolution');
# #   #isSchooling[which(isSchoolingN>7)]=TRUE;
# #   #isSchooling[which(isSchoolingN<=7)]=FALSE;
# #   
# #   plot(isSchooling)
# #   sum(isSchooling)
# #   #which(isSchooling)
# #   bouts = diff(which(isSchooling))-1
# #   sum(bouts>1)
# # #  toRet = data.frame(videoName = as.character(fileName), autolatency=min(which(isSchooling)) , autotime = sum(isSchooling), autobouts = sum(bouts>1));
#   
#  #######################non of the above criteria such as moving and direction are in use######################
# ########################################### ANOTHER WAY TO FIND SCHOOLING. IT IS BASED ON CLOSENESS FOR MORE THAN 3 SECONDS####
# 
#   tst = isClose & isFishMoving
#   tstAv=averageIt2(tst,30,20) # it was (tst,30,20)
# 
# #   tst2 = isFishMoving;
# #   tst2Av = averageIt2(tst2,30,15) # it was (tst,30,20)
# #   mv = rep(0,1,length(tst2Av));
# #   mv[tst2Av>0.5] = 1;
# #   print(tst2Av)
# #   
#   
#   sch = rep(0,1,length(tstAv))
#   sch[(tstAv>0.5)] = 1;
#   
#   #sch = sch & mv;
#   
#   print(sch)
#   
#   isSchooling = rep(0,1,length(sch));
#   
#   dsch = c(0,diff(sch));
#   starts = which(dsch==1);
#   stops = which(dsch==-1);
#   if(length(starts)<length(stops))
#   {
#     starts = c(1,starts);
#     
#   }else{
#     if (length(starts)>length(stops))
#       stops = c(stops,length(sch))
#   }
#   
#   filtered_starts = c();
#   filtered_stops = c();
#   
#   print(starts)
#   print(stops)
#   for (ii in 1:length(stops)) # remove very short schooling segments (less than 2 seconds..)
#   {
#     if ((stops[ii]-starts[ii])>2)
#     {
#       filtered_starts = c(filtered_starts,starts[ii])
#       filtered_stops = c(filtered_stops,stops[ii])
#       isSchooling[starts[ii]:stops[ii]] = 1;
#     } 
#   }
#   
#   
#   filtered_startsTMP = filtered_starts;
#   filtered_stopsTMP = filtered_stops;
#   print("Before:")
#   print (filtered_starts)  ;
#   print (filtered_stops);
#   
#   startsIdxToRemove = c();
#   stopsIdxToRemove = c();
#   
# 
# if (length(filtered_starts)>0)
# {
#   for (ii in 1:(length(filtered_starts)-1)) # remove very short schooling segments (less than 2 seconds..)
#   {
#     if (abs(filtered_stops[ii]-filtered_starts[ii+1])<2)
#     {
#       #filtered_startsTMP = filtered_startsTMP[-(ii+1)];
#       startsIdxToRemove = c(startsIdxToRemove,ii+1);
#       #filtered_stopsTMP = filtered_stopsTMP[-(ii)]
#       stopsIdxToRemove = c(stopsIdxToRemove,ii);
#     } 
#   }
# }
#   
# #   filtered_starts = filtered_startsTMP;
# #   filtered_stops = filtered_stopsTMP;
#   
#   print("after:")
#   #print (filtered_starts)  ;
#   #print (filtered_stops);
#   
#   print (startsIdxToRemove);
#   print (stopsIdxToRemove);
#   
#   
#   if (length(startsIdxToRemove)!=0)
#   {
#     filtered_starts = filtered_starts[-startsIdxToRemove];
#     filtered_stops = filtered_stops[-stopsIdxToRemove];
#   }
#   
#   print("after:")
#   print (filtered_starts)  ;
#   print (filtered_stops);
# #     
# #   print (startsIdxToRemove);
# #   print (stopsIdxToRemove);
#   
#   
# ##################################################################
#   outputInfo = data.frame(fishPosSec=fishPosSec,modelPosSec= modelPosSec, isSchooling= isSchooling, isClose=isClose, dist=dist, isFishMoving=isFishMoving);
#   startStop=data.frame(starts = filtered_starts, stops = filtered_stops);
#   write.csv(outputInfo,file=outputFileName,row.names = FALSE, col.names = FALSE)
#   
#   #write.csv(startStop,file=outputFileNameStartStops,row.names = FALSE, col.names = FALSE)
#   
#   if (length(filtered_starts)==0)
#   {
#     al_temp = 300;
#   }else{
#     al_temp = min(filtered_starts);
#     
#   }
#   toRet = data.frame(videoName = as.character(fileName), autolatency=al_temp , autotime = sum(isSchooling), autobouts = length(filtered_starts));
#   
#   
#   return(toRet);
# }

