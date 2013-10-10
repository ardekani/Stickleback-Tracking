######### TO TEST ############333
# from http://stats.stackexchange.com/questions/1142/simple-algorithm-for-online-outlier-detection-of-a-generic-time-series
tsoutliers <- function(x,plot=FALSE)
{
  x <- as.ts(x)
  if(frequency(x)>1)
    resid <- stl(x,s.window="periodic",robust=TRUE)$time.series[,3]
  else
  {
    tt <- 1:length(x)
    resid <- residuals(loess(x ~ tt))
  }

  resid.q <- quantile(resid,prob=c(0.25,0.75),na.rm = TRUE)
  iqr <- diff(resid.q)
  limits <- resid.q + 1.5*iqr*c(-1,1)
  score <- abs(pmin((resid-limits[1])/iqr,0) + pmax((resid - limits[2])/iqr,0))
  if(plot)
  {
    plot(x)
    x2 <- ts(rep(NA,length(x)))
    x2[score>0] <- x[score>0]
    tsp(x2) <- tsp(x)
    points(x2,pch=19,col="red")
    return(invisible(score))
  }
  else
    return(score)
}



###################### 
isThisLineOutlier <-function(data,index)
{
  origline = as.numeric(data[index,]);
  # now we collect all of the neighbor frames that have only 1 detected objects, use their average as the true point and pickup the correct measuremnet
  
  currentFrameNum = origline[1];
  winSize = 30;
  frameDiffThresh = 30;
  topIndx = index - winSize;
  if (topIndx<1)
  {
    topIndx =1;
  }
  botIndx = index + winSize;
  if(botIndx>max(dim(data)))
  {
    botIndx = max(dim(data));
  }
  searchWindow = data[topIndx:botIndx,];
  
  
  if(prod(dim(searchWindow))==0)
  {
    return(FALSE); # can not do the test
  }
  
  fnDiff = abs(searchWindow[,1] - currentFrameNum);
  searchWindow = searchWindow[fnDiff<frameDiffThresh,]
  
  if(prod(dim(searchWindow))==0)
  {
    return(FALSE); # can not do the test
  }
  
  
  #After making the current frame to only 1 measurement, we want to see if the measurement is an outlier or not

  
  currentFrameIndex = which(searchWindow[,1]==currentFrameNum);
  
#   print("currentFrameIndex");
#   print(currentFrameIndex);
#   print("searchWindow")
#   print(searchWindow)
  if(length(searchWindow[,5])<3)
  {
    return(TRUE)
    
  }
  scoreX = tsoutliers(searchWindow[,5]);
  scoreY = tsoutliers(searchWindow[,6]);
  score = scoreX + scoreY;
  ss = sum(is.na(score));

  if(ss>0) #THE SCORE VALUE FOR SMALL SIZE SEARCH WINDOW BECOMES NA OR INF
  {
    return(TRUE)
  }
  

  if(score[currentFrameIndex]!=0)
  {
    return(TRUE);
  }else
  {
    return(FALSE);
  }
  
}


#####################

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
  {
    topIndx =1;
  }
  botIndx = index + winSize;
  if(botIndx>max(dim(data)))
  {
    botIndx = max(dim(data));
  }
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
  #if the line is fixable, we want to eliminate the measurement that is not close to the neighbors
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

filterSticklebackTrackFile <- function(inFileName, firstFrame, lastFrame,max_Accpetable=2)
{
  if (!file.exists(inFileName))
  {
    sprintf("%s does not exist!",inFileName)
    return (FALSE)
  }
  
  n_col = count.fields(inFileName,sep=",");
  origTrackData = read.csv(inFileName,header=FALSE, fill = TRUE , col.names=1:max(n_col));
  
  #remove the lines with NA as "the number of detected components"
  filteredTrackData = origTrackData[complete.cases(origTrackData[,2]),];
  #remove frames with more than 2 measurement
  filteredTrackData = filteredTrackData[(filteredTrackData[,2]<=max_Accpetable),];
  #now we want to fix the frames that have two or more than two measurements, we want to pick the right one and then 
  

  
  for (j in 1:2) # go over the file twice - an arbitrary choice! TODO: it should repeat until there is no more fixing possible
  {

    morethanOneDetectedIndex = which(filteredTrackData[,2]>1);
    if(length(morethanOneDetectedIndex)>0)
    {
      for (i in 1:length(morethanOneDetectedIndex))
      {
        #ptm = proc.time();
        index = morethanOneDetectedIndex[i];
        fixedLine = fixThisLineWithMultipleMeasurements(filteredTrackData,index);
        filteredTrackData[index,] = fixedLine;
        #pttm = proc.time()-ptm;
        #print(pttm);
        #print(i)
      }
    }


    
    
  }

  # now remove the frames that are not fixable (after two rounds)

  
  filteredTrackData = filteredTrackData[(filteredTrackData[,2]==1),]; # keep only one measurement frames
  #now only pick the first component
  filteredTrackData = filteredTrackData[,1:8];
  

  
  outFileName=paste(substr(inFileName,1,nchar(inFileName)-4),"_filtered.txt",sep="");
  write.table(filteredTrackData,file = outFileName,sep = ",", row.names = FALSE, col.names = FALSE);
  
}

####################################
#smooth an already filtered file - this means only frames with one measurements are here and two/more measurements frames are fixed

###################################
smoothSticklebackTrackFile <-function (inFileName, firstFrame, lastFrame)
{
  
  outFileName=paste(substr(inFileName,1,nchar(inFileName)-4),"_smoothed.txt",sep="");
  
  nDim = 4;
  origTrackFile = read.csv(inFileName,header=FALSE)
  names(origTrackFile) = c("fn","NC","xdir","ydir","x","y","useless1","useless2");
  
  #first remove the outliers
  isOutlier = c();
  print("finding outliers ....")
  for( i in 1:max(dim(origTrackFile)))
  {
    tmp = isThisLineOutlier(origTrackFile,i);
    #print("-----------------ii = ----------------")
    #print(i)
    #print("---------------------------------------")
    isOutlier = c(isOutlier,tmp);  
  }
  
  origTrackFile = origTrackFile[-which(isOutlier),]
  
  #framesWithZeroComponents = which(origTrackFile$NC==0);
  #origTrackFile = origTrackFile[-framesWithZeroComponents,];
  
  totalNumOfEntries = length(origTrackFile$fn)
  prev_pos = as.numeric(origTrackFile[1,3:(nDim+2)]);
  prev_fn = origTrackFile$fn[1];
  smoothedData = c();
  smoothedData = rbind(smoothedData,c(prev_fn,prev_pos));
  preAllocatedSmoothData = matrix(data=NA,nrow=10000,ncol=5)
  preallocateCounter = 1;
  preAllocatedSmoothData[1,] = c(prev_fn,prev_pos);
  preallocateCounter = preallocateCounter + 1;
  for (ff in 2:(totalNumOfEntries-1)) 
  {
    #    curr_pos = c(origTrackFile$x[ff],origTrackFile$y[ff],origTrackFile$z[ff]);
    curr_pos = as.numeric(origTrackFile[ff,3:(nDim+2)]);
    curr_fn = origTrackFile$fn[ff];
    distanceOfCurrAndPrev = sqrt( (prev_pos[3]-curr_pos[3])^2 + (prev_pos[4]-curr_pos[4])^2 );
    
    if (curr_fn - prev_fn -1 > 0)# && (curr_fn - prev_fn -1 <10)
    {
      xdxd = round(seq(prev_pos[1],curr_pos[1], length.out = curr_fn - prev_fn -1),digits=2);
      ydyd = round(seq(prev_pos[2],curr_pos[2], length.out = curr_fn - prev_fn -1),digits=2);
      xx   = round(seq(prev_pos[3],curr_pos[3], length.out = curr_fn - prev_fn -1),digits=2);
      yy   = round(seq(prev_pos[4],curr_pos[4], length.out = curr_fn - prev_fn -1),digits=2);
      fnfn = (prev_fn+1):(curr_fn-1);
      tmp = cbind(fnfn,xdxd,ydyd,xx,yy);

      
#       smoothedData = rbind(smoothedData,tmp);
#       smoothedData = rbind(smoothedData,c(curr_fn,curr_pos));
      
      preAllocatedSmoothData[preallocateCounter:(preallocateCounter+length(fnfn)-1),] = tmp;
      preallocateCounter = preallocateCounter +length(fnfn);
      preAllocatedSmoothData[preallocateCounter,] = c(curr_fn,curr_pos);
      preallocateCounter = preallocateCounter + 1;
      

      
    } else {
      
#      smoothedData = rbind(smoothedData,c(curr_fn,curr_pos));
      preAllocatedSmoothData[preallocateCounter,] = c(curr_fn,curr_pos);
      preallocateCounter = preallocateCounter + 1;
      
    }
    
    prev_pos = curr_pos;      
    prev_fn = curr_fn;
  }
  
  
  # make sure to get rid of the frames that are out of (firstFrame, lastFrame)
  #print(dim(preAllocatedSmoothData));
  smoothedData = preAllocatedSmoothData[-which(is.na(preAllocatedSmoothData[,1])),];
  #print("dim smoothedData")
  #print(dim(smoothedData))
  allFrameNums = smoothedData[,1];
  toRemoveIndeces = c(which(allFrameNums<firstFrame), which(allFrameNums>lastFrame));
  #print("toRemoveIndeces")
  #print(toRemoveIndeces)
  smoothedData = smoothedData[-toRemoveIndeces,];
  
  write.table(smoothedData,file = outFileName,sep = ",", row.names = FALSE, col.names = FALSE);
  print("finished")
}
