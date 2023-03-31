using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}
char* imdb::binarySearch(const string& player,int leftIndex, int rightIndex) const{
  if(leftIndex > rightIndex) return NULL;
  int midIndex = (leftIndex + rightIndex) / 2;
  int midElement = *((int *)actorFile + midIndex);
  
  char * curr = (char*)actorFile + midElement;
  if(strcmp(player.c_str(), curr) == 0){
    return (char*)actorFile + midElement;
  } else if(strcmp(player.c_str(),curr) > 0){
    return binarySearch(player, midIndex+1,rightIndex);
  }
  return binarySearch(player,leftIndex,midIndex);  
}
// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const { 
    int leftIndex = 1;
    int rightIndex = *(int *)actorFile;
    char* myWord = binarySearch(player,leftIndex,rightIndex);
    if(myWord == NULL) return false;
    int fromStart = 0;
    int playerLength = player.size();
    if(player.size()%2 ==1){
      myWord += (playerLength + 1);
      fromStart+= (playerLength + 1);
    }else{
      myWord += (playerLength + 2);
      fromStart+= (playerLength + 2);
    }
    int nMovieByte = 2;
    short numberMovies = * (short *) myWord;
    fromStart +=nMovieByte;
    myWord += nMovieByte;
    if(fromStart % 4 != 0){
      myWord += nMovieByte;
    }
    int movieBytes = 4;
    for(short i = 0; i<numberMovies; i++){
      film myMovie;
      char* movieName = (char*) movieFile + * (int *) myWord;
      myWord+= movieBytes;
      string title; 
      title = string(movieName);
      int releaseYear = 1900 + *(movieName + title.size()+1);
      myMovie.title = title;
      myMovie.year = releaseYear;
      films.push_back(myMovie);
    }
  return true; 
}
char* imdb :: movieBSearch(const film& movie, int leftIndex, int rightIndex) const{
  if(leftIndex > rightIndex) return NULL;
  int midIndex = (leftIndex + rightIndex) / 2;
  int midElement = *((int *)movieFile+ midIndex);
  char* currentMovie = (char*)movieFile + midElement;
  film myMovie;
  myMovie.title = string(currentMovie);
  myMovie.year = 1900 + *(currentMovie + myMovie.title.size() +1);
  if(myMovie == movie){
    return currentMovie;
  }else if(myMovie < movie){
    return movieBSearch(movie,midIndex +1, rightIndex);
  }else if (leftIndex==rightIndex){
      return NULL;
  }
   return movieBSearch(movie, leftIndex, midIndex);
}
bool imdb::getCast(const film& movie, vector<string>& players) const { 
    int leftIndex = 1;
    int rightIndex = *(int *) movieFile;
    char* myMovie = movieBSearch(movie, leftIndex, rightIndex);
    if(myMovie==NULL) return false; 
    int distance = 0;
    myMovie+= (movie.title.size() + 2);
    distance += movie.title.size() +2;
    if(distance % 2 == 1){
      myMovie += 1;
      distance +=1;
    }
    short nActor = * (short *) myMovie;
    distance += 2;
    myMovie += 2;
    if(distance % 4 != 0){
      myMovie += 2;
    }
    int stepActor = 4;
    for(short i=0; i<nActor; i++){
        char* movieActor = (char*) actorFile + *(int *) myMovie;
        myMovie += stepActor;
        string actorName = movieActor;
        players.push_back(actorName);
    }
    return true;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}


