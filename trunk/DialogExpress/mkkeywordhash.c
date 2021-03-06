/*
** Compile and run this standalone program in order to generate code that
** implements a function that will translate alphabetic identifiers into
** parser token codes.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
** All the keywords of the SQL language are stored as in a hash
** table composed of instances of the following structure.
*/
typedef struct Keyword Keyword;
struct Keyword {
  char *zName;         /* The keyword name */
  char *zTokenType;    /* Token value for this keyword */
  int mask;            /* Code this keyword if non-zero */
  int id;              /* Unique ID for this record */
  int hash;            /* Hash on the keyword */
  int offset;          /* Offset to start of name string */
  int len;             /* Length of this keyword, not counting final \000 */
  int prefix;          /* Number of characters in prefix */
  int iNext;           /* Index in aKeywordTable[] of next with same hash */
  int substrId;        /* Id to another keyword this keyword is embedded in */
  int substrOffset;    /* Offset into substrId for start of this keyword */
};

/*
** These are the keywords
*/
static Keyword aKeywordTable[1000];

/* Number of keywords */
static int NKEYWORD = (sizeof(aKeywordTable)/sizeof(aKeywordTable[0]));

/* An array to map all upper-case characters into their corresponding
** lower-case character. 
*/
const unsigned char UpperToLower[] = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
     18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
     36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53,
     54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99,100,101,102,103,
    104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,
    122, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,104,105,106,107,
    108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
    126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
    144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,
    162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,
    180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,
    198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,
    216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,
    234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,
    252,253,254,255
};
#define UpperToLower UpperToLower

/*
** Comparision function for two Keyword records
*/
static int keywordCompare1(const void *a, const void *b){
  const Keyword *pA = (Keyword*)a;
  const Keyword *pB = (Keyword*)b;
  int n = pA->len - pB->len;
  if( n==0 ){
    n = strcmp(pA->zName, pB->zName);
  }
  return n;
}
static int keywordCompare2(const void *a, const void *b){
  const Keyword *pA = (Keyword*)a;
  const Keyword *pB = (Keyword*)b;
  int n = strcmp(pA->zName, pB->zName);
  return n;
}
static int keywordCompare3(const void *a, const void *b){
  const Keyword *pA = (Keyword*)a;
  const Keyword *pB = (Keyword*)b;
  int n = pA->offset - pB->offset;
  return n;
}

/*
** Return a KeywordTable entry with the given id
*/
static Keyword *findById(int id){
  int i;
  for(i=0; i<NKEYWORD; i++){
    if( aKeywordTable[i].id==id ) break;
  }
  return &aKeywordTable[i];
}

/*
** This routine does the work.  The generated code is printed on standard
** output.
*/
int main(int argc, char **argv){
  int i, j, k, h;
  int bestSize, bestCount;
  int count;
  int nChar;
  int aHash[1000];  /* 1000 is much bigger than NKEYWORD */
  char aBuf[1024];
  char *p, *s1, *s2;
  /* Remove entries from the list of keywords that have mask==0 */
  /*for(i=j=0; i<NKEYWORD; i++){
    if( aKeywordTable[i].mask==0 ) continue;
    if( j<i ){
      aKeywordTable[j] = aKeywordTable[i];
    }
    j++;
  }*/
  for (i=j=0; i<NKEYWORD && gets(aBuf); i++){
    if (!aBuf[0] || aBuf[0] == ';') continue;
    s2=0;
    for(s1=p=aBuf; *p && *p!=' ' && *p!='\t'; p++);
    if (*p){
      *p='\0';
      for(s2=++p;*s2 && (*s2==' ' || *s2=='\t'); s2++);
      //*s2='\0';
    }
    if (!s2 || !*s2 || *s2==' ' || *s2 == '\t'){ 
      s2=++p;
      sprintf(s2, "TK_%s\0", s1);
    }
    if (*s1 && s2 && *s2)
    {
      Keyword *p = &aKeywordTable[j];
      p->zName = strdup(s1);
      p->zTokenType = strdup(s2);
      p->mask = 1;
      fprintf(stderr, "%s=%s\n", s1, s2);
      j++;
    }
  }
  NKEYWORD = j;

  /* Fill in the lengths of strings and hashes for all entries. */
  for(i=0; i<NKEYWORD; i++){
    Keyword *p = &aKeywordTable[i];
    p->len = strlen(p->zName);
    p->hash = (UpperToLower[p->zName[0]]*4) ^
              (UpperToLower[p->zName[p->len-1]]*3) ^ p->len;
    p->id = i+1;
  }

  /* Sort the table from shortest to longest keyword */
  qsort(aKeywordTable, NKEYWORD, sizeof(aKeywordTable[0]), keywordCompare1);

  /* Look for short keywords embedded in longer keywords */
  for(i=NKEYWORD-2; i>=0; i--){
    Keyword *p = &aKeywordTable[i];
    for(j=NKEYWORD-1; j>i && p->substrId==0; j--){
      Keyword *pOther = &aKeywordTable[j];
      if( pOther->substrId ) continue;
      if( pOther->len<=p->len ) continue;
      for(k=0; k<=pOther->len-p->len; k++){
        if( memcmp(p->zName, &pOther->zName[k], p->len)==0 ){
          p->substrId = pOther->id;
          p->substrOffset = k;
          break;
        }
      }
    }
  }

  /* Sort the table into alphabetical order */
  qsort(aKeywordTable, NKEYWORD, sizeof(aKeywordTable[0]), keywordCompare2);

  /* Fill in the offset for all entries */
  nChar = 0;
  for(i=0; i<NKEYWORD; i++){
    Keyword *p = &aKeywordTable[i];
    if( p->offset>0 || p->substrId ) continue;
    p->offset = nChar;
    nChar += p->len;
    for(k=p->len-1; k>=1; k--){
      for(j=i+1; j<NKEYWORD; j++){
        Keyword *pOther = &aKeywordTable[j];
        if( pOther->offset>0 || pOther->substrId ) continue;
        if( pOther->len<=k ) continue;
        if( memcmp(&p->zName[p->len-k], pOther->zName, k)==0 ){
          p = pOther;
          p->offset = nChar - k;
          nChar = p->offset + p->len;
          p->zName += k;
          p->len -= k;
          p->prefix = k;
          j = i;
          k = p->len;
        }
      }
    }
  }
  for(i=0; i<NKEYWORD; i++){
    Keyword *p = &aKeywordTable[i];
    if( p->substrId ){
      p->offset = findById(p->substrId)->offset + p->substrOffset;
    }
  }

  /* Sort the table by offset */
  qsort(aKeywordTable, NKEYWORD, sizeof(aKeywordTable[0]), keywordCompare3);

  /* Figure out how big to make the hash table in order to minimize the
  ** number of collisions */
  bestSize = NKEYWORD;
  bestCount = NKEYWORD*NKEYWORD;
  for(i=NKEYWORD/2; i<=2*NKEYWORD; i++){
    for(j=0; j<i; j++) aHash[j] = 0;
    for(j=0; j<NKEYWORD; j++){
      h = aKeywordTable[j].hash % i;
      aHash[h] *= 2;
      aHash[h]++;
    }
    for(j=count=0; j<i; j++) count += aHash[j];
    if( count<bestCount ){
      bestCount = count;
      bestSize = i;
    }
  }

  /* Compute the hash */
  for(i=0; i<bestSize; i++) aHash[i] = 0;
  for(i=0; i<NKEYWORD; i++){
    h = aKeywordTable[i].hash % bestSize;
    aKeywordTable[i].iNext = aHash[h];
    aHash[h] = i+1;
  }

  /* Begin generating code */
  printf("/* Hash score: %d */\n", bestCount);
  printf("static int keywordCode(const char *z, int n){\n");

  printf("  static const char zText[%d] =\n", nChar+1);
  for(i=j=0; i<NKEYWORD; i++){
    Keyword *p = &aKeywordTable[i];
    if( p->substrId ) continue;
    if( j==0 ) printf("    \"");
    printf("%s", p->zName);
    j += p->len;
    if( j>60 ){
      printf("\"\n");
      j = 0;
    }
  }
  printf("%s;\n", j>0 ? "\"" : "  ");

  printf("  static const unsigned char aHash[%d] = {\n", bestSize);
  for(i=j=0; i<bestSize; i++){
    if( j==0 ) printf("    ");
    printf(" %3d,", aHash[i]);
    j++;
    if( j>12 ){
      printf("\n");
      j = 0;
    }
  }
  printf("%s  };\n", j==0 ? "" : "\n");    

  printf("  static const unsigned char aNext[%d] = {\n", NKEYWORD);
  for(i=j=0; i<NKEYWORD; i++){
    if( j==0 ) printf("    ");
    printf(" %3d,", aKeywordTable[i].iNext);
    j++;
    if( j>12 ){
      printf("\n");
      j = 0;
    }
  }
  printf("%s  };\n", j==0 ? "" : "\n");    

  printf("  static const unsigned char aLen[%d] = {\n", NKEYWORD);
  for(i=j=0; i<NKEYWORD; i++){
    if( j==0 ) printf("    ");
    printf(" %3d,", aKeywordTable[i].len+aKeywordTable[i].prefix);
    j++;
    if( j>12 ){
      printf("\n");
      j = 0;
    }
  }
  printf("%s  };\n", j==0 ? "" : "\n");    

  printf("  static const unsigned short int aOffset[%d] = {\n", NKEYWORD);
  for(i=j=0; i<NKEYWORD; i++){
    if( j==0 ) printf("    ");
    printf(" %3d,", aKeywordTable[i].offset);
    j++;
    if( j>12 ){
      printf("\n");
      j = 0;
    }
  }
  printf("%s  };\n", j==0 ? "" : "\n");

  printf("  static const unsigned char aCode[%d] = {\n", NKEYWORD);
  for(i=j=0; i<NKEYWORD; i++){
    char *zToken = aKeywordTable[i].zTokenType;
    if( j==0 ) printf("    ");
    printf("%s,%*s", zToken, (int)(14-strlen(zToken)), "");
    j++;
    if( j>=5 ){
      printf("\n");
      j = 0;
    }
  }
  printf("%s  };\n", j==0 ? "" : "\n");

  printf("  int h, i;\n");
  //printf("  if( n<2 ) return TK_ID;\n");
  printf("  h = ((UpperToLower[((unsigned char*)z)[0]]*4) ^\n"
         "      (UpperToLower[((unsigned char*)z)[n-1]]*3) ^\n"
         "      n) %% %d;\n", bestSize);
  printf("  for(i=((int)aHash[h])-1; i>=0; i=((int)aNext[i])-1){\n");
  printf("    if( aLen[i]==n &&"
                   " strncasecmp(&zText[aOffset[i]],z,n)==0 ){\n");
  printf("      return aCode[i];\n");
  printf("    }\n");
  printf("  }\n");
  printf("  return TK_ID;\n");
  printf("}\n");

  return 0;
}
