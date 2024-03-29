
#include <iostream>
#include <fstream>
#include <iomanip>
#include <limits>
#include <cstring>

using namespace std;

uint8_t SOI[]  = {0xFF, 0xD8};  // start of image tag

int main (int argc, char *argv[])
{
  if ((argc < 2) || (argc > 3))
  {
    cerr << "Usage: "  << argv[0] << " infile.jpg [outfile.jpg]" << endl;
    return 1;
  }

  string inFileName = argv[1];
  ifstream inFile;
  
  inFile.open (inFileName, ifstream::in | ifstream::binary);
  if (!inFile.is_open())
  {
    cerr << "failed to open \""  << inFileName << "\"" <<  endl;
    return 1;
  }
  
  // detect file size bz reading through file
  inFile.seekg(0, std::ios_base::beg);
  inFile.ignore(std::numeric_limits<std::streamsize>::max());
  std::streamsize inFileSize = inFile.gcount();
  inFile.clear();
  inFile.seekg(0, std::ios_base::beg);

  if (inFileSize < (sizeof(SOI) ))
  {
    cerr << "File size too small for a JPEG image" << endl;
    return 1;
  }
  
  char* fileContent = new char [inFileSize];

  inFile.read (fileContent, inFileSize);
  if (inFile.bad())
  {
    cerr << "Error reading from \""  << inFileName << "\"" <<  endl;
    return 1;
  }
  inFile.close();
 
  static_assert(sizeof(std::streamsize) <= sizeof(int64_t), "std::streamsize > 64 bit not supported");

  // check for starting tag of the image
  if (memcmp (fileContent, SOI, sizeof(SOI) ) != 0)
  {
    cerr << "SOI (Start Of Image) tag not found, probably no JFIF file." <<  endl;
    return 1;
  }

  // count zero bytes from the end
  int64_t zeroCnt=0;
  int64_t pos = (int64_t)inFileSize - 1;

  while (pos > 0 && fileContent[pos] == 0)
  {
    pos--;
    zeroCnt ++;
  }
  
  bool endFound = false;
  if (pos>1)
  {
    endFound = ((unsigned char)(fileContent[pos]) == 0xD9) && ((unsigned char)(fileContent[pos-1]) == 0xFF);
  }
  
  if (argc == 3)
  {
    // write output file, if two arguments given
    if (endFound)
    {
      string outFileName = argv[2];
      ofstream outFile;

      if ((inFileName == outFileName) && (zeroCnt == 0))
      {
        cout << "No trailing zeros found, not overwriting input " << inFileName << endl;
      }
      else
      {
        outFile.open(outFileName, ofstream::out | ofstream::binary);
        if (!outFile.is_open())
        {
          cerr << "failed to open \"" << outFileName << "\"" << endl;
          return 1;
        }

        outFile.write(fileContent, pos + 1);
        if (outFile.bad())
        {
        cerr << "error writing to \"" << outFileName << "\"" << endl;
        return 1;
        }
        outFile.close();
        cout << "Saved " << zeroCnt << " bytes in \"" << outFileName << "\"" << endl;
      }
    }
    else
    {
      cerr << "EOI tag not found in \"" << inFileName << "\". Not writing to output file. Maybe no JFIF file." <<  endl;
    }
  }
  else
  {
    // print some info
    if (endFound)
    {
      cout << "File size       : "  << setw(10) << inFileSize << " bytes" <<  endl;
      cout << "Number of zeros : "  << setw(10) << zeroCnt <<  endl;
    }
    else
    {
      cerr << "EOI tag not found in \"" << inFileName << "\". Maybe no JFIF file." <<  endl;
    }
  }
 
  delete[] fileContent;
  return 0;
}

