/*
To run the pipeline over a csv file that contains video file names with some other info
Author: Reza Ardekani
Date : 08/04/2013
*/

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
void tokenizeCSVString(std::string inp,std::vector<std::string> &tokens)
{
	std::string token;
    std::istringstream iss(inp);
	   while ( getline(iss, token, ',') )
	   {
//		  std::cout << token << std::endl;
		  tokens.push_back(token);
	   }

}
int main()
{

	//std::string listOfVideos = "G:\\Peichel\\PJxPJ_F2_2011\\school_only\\listoffiles.txt";

	std::string listOfVideos = "C:\\Users\\Reza\\Dropbox\\batch processing of SticklebackVideos\\correctingFileNames\\videoinfo1of4_PJxPJ_F2_2011_schooling_only_test.txt";
	std::string listOfFailedVideo = listOfVideos.substr(0,listOfVideos.length()-4);
	listOfFailedVideo+="_failed.txt";
	FILE* fout = NULL;
	fout= fopen(listOfFailedVideo.c_str(),"w");
	if(fout!=NULL)
		printf("\nCan not write the list of failed videos");

	std::string line;
	std::ifstream infile (listOfVideos.c_str());
	std::ofstream outfile (listOfFailedVideo.c_str());
	if (infile.is_open())
	{
		while ( infile.good() )
		{
			getline (infile,line);
			//std::cout<<line;
			char fn[200];
			int startFrame=0;int endFrame=0;int matchFrame=0;
			sscanf(line.c_str(),"%s,%d,%d,%d\n",fn,startFrame,endFrame,matchFrame);
			std::vector<std::string> tkns;
			tokenizeCSVString(line,tkns);

			tkns[0]; //filename
			tkns[0] = ""
			tkns[1]; //startframe
			tkns[2]; //endframe
			tkns[3]; //matchframe

			char command[200];
			sprintf("command",






		}
		infile.close();
	}
	else std::cout << "Unable to open file"; 
	if(fout!=NULL)
		fclose(fout);
	getchar();
	return 0;





	char myString[] = "The quick brown fox";
char *p = strtok(myString, " ");
while (p) {
    printf ("Token: %s\n", p);
    p = strtok(NULL, " ");
}



	return 0;
}
