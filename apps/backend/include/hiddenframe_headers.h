#ifndef HIDDENFRAME_HEADERS_H
#define HIDDENFRAME_HEADERS_H

#include <string>
#include <vector>
#include <sqlite/sqlite3.h>
#include <crow/json.h>

using namespace std;

const int MAX_SIZE = 500000;

class image
{
public:
    image();
    image(string filepath);
    image(const unsigned char *Data, long long unsigned int length, string ext);
    ~image();
    int width;
    int height;
    int channels;
    string filetype;
    void displayImageProperties();
    void modify_image(int n, string payload);
    string retrieve_payload(int n);
    void write_image(string filename);
    bool valid_image();
    // Calculates the ideal skip size for an image.
    int calculateSkipSize(int imageSize, int messageSize);
    // Generates a valid key for an image of the form (a_1,b_1,a_2,b_2,e,f) where the ab pairs
    // may be multiplied together mod imageSize so that a*b mod imageSize = idealSkipSize;
    string generateKey(int imageSize, int messageSize, int channels);
    // Decodes an image key.  Returns the ideal skip size or -1 if the key is invalid.
    int decodeKey(string key, int imageSize);
    // Generates a dummy key that does not meet the requirements of a valid key though looks like a valid
    // key anyone unfamiliar with how to decode a key.
private:
    unsigned char *original_image;
    unsigned char *modified_image;
    vector<int> coprimes;
    void load_image(string filepath);
    // To find the multiplicative inverse of an element a from an Z_n group.
    long long findInverse(long long a, int n);
    // To find a number b, such that, g=ab mod n, where g is a generator of a Z_n group and
    // the ideal skip size of when encoding or decoding an image, a is also a generator
    // of the same Z_n group, an a inverse is a^-1 such that a*a^-1 mod n = 1.
    long long findB(long long idealSkipSize, int n, int aInverse);
    // Performs the Euclidean Algorithm to find gcd(n,b), by recursivly using the property
    // of the division algorithm that is, for n>b, n=qb + r for some q and r in Z where
    // 0<=r<b and gcd(n,b)=gcd(b,r)
    int euclideanAlgorithm(int n, int b);
    // Iteratively uses the Euclidean Algorithm on a value n to find value in
    //{0,1,2,3,...,n-1} that are coprime to n and store them in an array, and keeps
    // track of the count of the number of values it finds.
    void coprime_numbers(int n);
    // Performs a binary search of the coprimes array to find an ideal skip size between pixels for
    // A payload going into an image.
    int binarySearch(int size, const int idealSkipSize);
    string generateDummyKey(int imageSize);
    // Compresses a string of binary bits into an array of integers with the format:
    //[3,1,2,0,4,1,...] that is to be read, "Three 1's, two 0's, four 1's..."
    vector<char> bitStringCompressor(int channels, string toCompress);
};

sqlite3 *createDB(const string &filepath);
void createNewAdmin(const string &username, const string &password);
void createNewUser(const string &username, const string &password, const int64_t InviteID);
bool authenticateUser(const string &username, const string &password);
bool changePassword(const string &username, const string &newPassword);
int64_t createInvite(const string &username, int64_t id);
vector<crow::json::wvalue> listInvites(const int &userId);
pair<int, string> verifyTokenWithDb(const string &tokenId);
void saveToken(const string &username, const string &tokenId);
void closeDB(sqlite3 *db);
bool usernameExists(const string &username);

#endif // HIDDENFRAME_HEADERS_H