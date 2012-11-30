#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <curl/curl.h>
 
struct MemoryStruct {
  char *memory;
  size_t size;
};
 
typedef struct quote_ {
  float open;
  float close;
} quote;

quote get_quote(char *uri);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

#ifdef MAIN
int main(void)
{
  quote my_quote;

  // Change MSFT to whatever stock symbol you want.
  my_quote = get_quote("http://finance.yahoo.com/d/quotes.csv?s=MSFT&f=sop");

  printf("Quote open:%f\nQuote close: %f\n", my_quote.open, my_quote.close);

  return 0;
}
#endif

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    exit(EXIT_FAILURE);
  }
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}


quote get_quote(char *uri) {
  CURL *curl;
  CURLcode res;
  char *data;
  quote my_quote;
 
  struct MemoryStruct chunk;
 
  chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
  chunk.size = 0;    /* no data at this point */ 
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, uri);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
 
    /* send all data to this function  */ 
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    
    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      return (quote){-1.0,-1.0};


    printf("%lu bytes retrieved\n%s", (long)chunk.size, (char *)chunk.memory);
    

    data = strtok(chunk.memory, ",");
    int cnt=0;
    do {
      switch(cnt++) {
      case 1:
	my_quote.open = atof(data);
      case 2:
       	my_quote.close = atof(data); 
      } 
     
      printf("%s\n",data);

    } while(data = strtok(NULL, ","));
    

    if(chunk.memory)
      free(chunk.memory);


    /* always cleanup */ 
    curl_easy_cleanup(curl);
  } else 
      return (quote){-1.0,-1.0};
  

  return (quote)(my_quote);
}
