#define DEBUG 1
#define REPORT 1


static void PrintDebug();
void PrintReport(const char * c ,const char * error_code,const char * c1 ,int error_code1);
void PrintReport(const char * c ,int error_code);
void PrintReport(const char * c);

void PrintError(const char * c ,int error_code); // Prints Error Code in errno regiester to terminal with COUT