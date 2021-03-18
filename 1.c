#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#define constSize 5
typedef struct array
{
    char *start; // pointer to the beginning of line 
    char *end; // pointer to end of line
    int len1; // allocated size 
    int len; // used size
} array;
// parsing variables 
struct globalArgs_t
{
    char *input;
    char *output;
    int l;
    int a;
    int n;
    int r;
} globalArgs;
char *program_name;
FILE *in;
FILE *out;
// initial memory allocation for a string 
void Create_array(array *);
// appending data to the end of a line 
void Add_to_array(array *, char *, long);
// freeing memory 
void Free_array(array *);
// showing the manual and errors
void Show_err( char* );
// parsing input and output file + option -h 
void Parsing( int, char ** );
// Reading file
int ReadFile(FILE *, array *);
// Deleting an list 
void Drop(array *);
// Print a list of string
void print(FILE *, array *);
// alphabetical and length comparison
int alp_len(const void* a, const void* b);

int len_comp(const void* a, const void *b);

int main( int argc, char **argv )
{
    program_name = argv[0];
    Parsing(argc, argv);
    array dict;
    Create_array(&dict);
    int count = ReadFile(in, &dict);
    if (globalArgs.n)
    {
        qsort(dict.start, count, sizeof(array**),len_comp);
    }
    else if (globalArgs.a)
    {
        qsort(dict.start, count, sizeof(array**),alp_len);
    }
    printf("\n");
    print(out, &dict);
    Drop(&dict);
    Free_array(&dict);
    return EXIT_SUCCESS;
}
int len_comp(const void* a, const void* b)
{
    const array* k=(const array*)a;
    const array* m=(const array*)b;
    return (globalArgs.r ? -1: 1)*(k->len - m->len);
}
int alp_len(const void *a, const void* b)
{
    const array* k=(const array*)a;
    const array* m=(const array*)b;
    int res = len_comp(k,m);
    if (!res) res = strcmp(k-> start , m-> start);
    return (globalArgs.r ? -1: 1)*res;
}
int ReadFile(FILE *stream, array *buf)
{
    /* copy to buf until EOF  */
    int len = 0;
    char c = EOF;
    array *tmp = NULL;
    do {
      c = fgetc(stream);
      if (c != EOF)
      {
        if (isprint(c))
        {
          if (!tmp)
          {
            tmp = calloc(1, sizeof(array));
            Create_array(tmp);
          }
          if (tmp)
          {
            Add_to_array(tmp, &c, 1);
          }
        }
        else
        {
          if (tmp)
          {
            LAST_TRY:
            if (tmp->len >= globalArgs.l)
            {
              Add_to_array(buf, tmp->start, tmp->len);
              len++;
            }
            else
            {
              Free_array(tmp);
              free(tmp);
            }
            tmp = NULL;
          }
        }
      }
    } while (c != EOF);
    if (tmp) goto LAST_TRY;
    return len;
}
void Drop(array * buf)
{
    for (char *cur_string = buf->start; cur_string < buf->end; cur_string++)
    {
      Free_array(cur_string);
      free(cur_string);
    }
}
void print(FILE *stream, array *buf)
{
    for (array **cur_string = buf->start; cur_string < buf->end; cur_string++)
    {
      fwrite((*cur_string)->start, sizeof(char), (*cur_string)->len,stream);
      fwrite("\n", sizeof(char), 1, stream);
    }
}
void Show_err( char* error )
{
    /*
     displays usage to stdin or stderr 
    */
    fprintf( error ? stderr : stdout, "%sUsage: %s [options][input_file [output_file]]\n"
    "\tOptions:\n"
    "\t\t-l - minimal array length, if less, ignore\n"
    "\t\t-a - sort by alphabet\n"
    "\t\t-n - sort by length\n"
    "\t\t-r - descending sort (only with -a or -n)\n"
    , (error ? error : ""), program_name);
    exit( error ? EXIT_FAILURE : EXIT_SUCCESS );
}
void Parsing( int argc, char **argv )
{
  /*
  Parse the options and parameters
  */
  const char *optString = "l:anrð?"; // this means options f and some of its value, r and value, i without value (flag), h flag, unknown 
  int opt = 0;
  globalArgs.input = NULL;
  globalArgs.output = NULL;
  globalArgs.l = 0;
  globalArgs.a = 0;
  globalArgs.n = 0;
  globalArgs.r = 0;
  // Parse options
  do
  {
    switch( opt )
    {
      case 'l':
      if (!sscanf(optarg, "%d", &globalArgs.l) || globalArgs.l < 0)
      Show_err("Error: length must be positive integer\n");
      break;
      case 'a':
      globalArgs.a = 1;
      break;
      case 'n':
      globalArgs.n = 1;
      break;
      case 'r':
      globalArgs.r = 1;
      break;
      case 'h':
      Show_err(0);
      break;
      case '?':
      Show_err("Error: cannot parse the options\n");
      break;
    }
    opt = getopt( argc, argv, optString );
  } while( opt != -1 );
  // Parse positional parameters
  // åñëè óêàçàëè ôàéëû
  if (argc >= optind)
  globalArgs.input = argv[optind];
  if (argc > optind)
  globalArgs.output = argv[optind + 1];
  // assignment of input and output streams 
  if (globalArgs.input)
    in = fopen(globalArgs.input, "rb");
  else
    in = stdin;
  if (globalArgs.output)
    out = fopen(globalArgs.output, "wb");
  else
    out = stdout;
  if (globalArgs.r && !globalArgs.n && !globalArgs.a)
      Show_err("Error: option -r can be used only with -n or - a\n");
  if (!in || !out)
      Show_err("Error: cannot open the file\n");
}
void Add_to_array(array *dest, char* src, long len)
{
  if (dest->len + len > dest->len1)
  { // checking "is there enough memory?" and re-allocation if not 
    dest->start = (char*)realloc(dest->start, sizeof(char) * (dest->len + len + constSize));
    if (!dest->start)
      exit( EXIT_FAILURE );
    dest->end = dest->start + dest->len; // changing pointers, because after reallocation the address of the string may change 
    dest->len1 = dest->len + len + constSize;
  }
    memcpy(dest->end, src, len); // appending data to the end 
    dest->len += len;
    dest->end += len;
}
void Create_array(array *s)
{
    s->start = (char*)malloc(constSize);
    if (!s->start)
      exit( EXIT_FAILURE );
    s->end = s->start;
    s->len1 = constSize;
    s->len = 0;
}
void Free_array(array *s)
{
    free(s->start);
}
