
#define CURL_STATICLIB
#define no_init_all
#include <curl/curl.h>

#include <bzlib.h>

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <ctime>
#include <vector>
#include <algorithm>
#include <execution>
#include <regex>
#include <tchar.h>
#include <windows.h>

#pragma warning(disable : 4996)

bool FORCE_DEBUG = false;

/* PASS Format : 0000,000000 */
bool is_valid_passport(const std::string &str)
{
    static const std::regex r(R"(\d{4},\d{6})");
    return regex_match(str, r);
}

void writeToFile(const std::vector<std::string> &data, const std::string &filePath)
{
    std::ofstream out;
    out.open(filePath, std::ofstream::out | std::ofstream::trunc);

    for (auto &item : data)
        out << item << std::endl;

    out.close();
}

/* Writes curl response to string */
static size_t getResponseToString(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

/* Function to download into FILE with curl */
size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

/* Download from url into file */
void downloadToFile(std::string url, const char *path)
{
    CURL *curl = curl_easy_init();
    FILE *file = fopen(path, "wb");

    if (url.size() > 0)
    {
        std::cout << url.c_str() << std::endl;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        CURLcode response = curl_easy_perform(curl);
    }

    curl_easy_cleanup(curl);
    fclose(file);
}

/* Should pase http to get link to .bz2 archive */
std::string parseUrl()
{
    CURL *curl;
    CURLcode response;
    std::string strResponse;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://xn--b1afk4ade4e.xn--b1ab2a0a.xn--b1aew.xn--p1ai/info-service.htm?sid=2000");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getResponseToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strResponse);
    response = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    static const std::regex reg(R"(https:.*[.]bz2)");
    std::smatch m;
    bool result;
    result = regex_search(strResponse, m, reg);
    if (result)
    {
        std::cout << m[0] << std::endl;
        return m[0];
    }
    return std::string();
}

/* Should encode cyrillic */
std::string encodeUrlHttps(std::string url)
{
    CURL *curl = curl_easy_init();
    std::string delimiter = "/";
    std::string encodedUrl("https:/");

    size_t pos = 0;
    std::string token;

    while ((pos = url.find(delimiter)) != std::string::npos)
    {
        token = url.substr(0, pos);
        if (token != "https:")
        {
            std::string part((curl_easy_escape(curl, token.c_str(), token.size())));
            encodedUrl.append(part);
            encodedUrl.append("/");
        }
        url.erase(0, pos + delimiter.length());
    }
    encodedUrl.append(url);
    curl_easy_cleanup(curl);
    return encodedUrl;
}

/* Reads .bz2 archive into result */
void readBZ2(const char *filePath, std::vector<std::string> &result)
{
    FILE *f;
    BZFILE *b;
    int nBuf = 0;
    long bufSize = 4096;
    unsigned char buf[4096];
    int bzerror = BZ_OK;
    int nWritten = 0;

    std::string line;
    line.reserve(20);

    f = fopen(filePath, "rb");
    rewind(f);

    if (!f)
    {
        std::cout << "No file" << std::endl;
        return;
    }

    b = BZ2_bzReadOpen(&bzerror, f, 0, 0, NULL, 0);

    if (bzerror != BZ_OK)
    {
        BZ2_bzReadClose(&bzerror, b);
        std::cout << "Error Read Open" << std::endl;
        fclose(f);
        return;
    }

    bzerror = BZ_OK;
    while (bzerror == BZ_OK)
    {
        nBuf = BZ2_bzRead(&bzerror, b, buf, sizeof buf);

        for (int i = 0; i < nBuf; i++)
        {
            if (buf[i] == '\n' || buf[i] == '\r')
            {
                if (is_valid_passport(line) || result.size() == 0)
                    result.push_back(line);

                line.clear();
                continue;
            }
            line.push_back(buf[i]);
        }
    }

    if (is_valid_passport(line))
        result.push_back(line);
    line.clear();
    line.shrink_to_fit();

    if (bzerror != BZ_STREAM_END)
    {
        std::cout << "Other Error : " << bzerror << std::endl;
        BZ2_bzReadClose(&bzerror, b);
    }
    else
        BZ2_bzReadClose(&bzerror, b);

    fclose(f);

    std::cout << "Lines read : " << result.size() << std::endl;
}

/* Compares 2 sorted vectors with passports
 and places uniq values from first file into firstUniq,
  and uniq values from second file into secondUniq  */
void compareFiles(const std::vector<std::string> &sortedFile1, const std::vector<std::string> &sortedFile2,
                  std::vector<std::string> &firstUniq, std::vector<std::string> &secondUniq)
//  std::vector<std::string> &pairs)
{
    std::string prevNum;
    prevNum.reserve(16);
    std::cout << "Set search started" << std::endl;
    for (int i = 0, j = 0; i < sortedFile2.size() && j < sortedFile1.size(); i++, j++)
    {
        if (sortedFile2[i] == sortedFile1[j])
        {
            // pairs.push_back(sortedFile2[i]);

            prevNum = sortedFile2[i];

            continue;
        }
        else if (sortedFile2[i] < sortedFile1[j])
        {
            --j;

            if (sortedFile2[i] != prevNum)
                secondUniq.push_back(sortedFile2[i]);

            continue;
        }
        else
        {
            --i;
            if (sortedFile1[j] != prevNum)
                firstUniq.push_back(sortedFile1[j]);
            continue;
        }
    }
    std::cout << "Set search finished" << std::endl;
}

int main()
{
    if (FORCE_DEBUG)
    {

        std::string urlFromSite;
        std::string urlFromSiteEncoded;
        urlFromSite = parseUrl();
        urlFromSiteEncoded = encodeUrlHttps(urlFromSite);
        downloadToFile(urlFromSiteEncoded, "from_site.csv.bz2");
        /*
        ????
        Parsing https link from site and downloading it does't work for some reason
        */
        return 0;
    }

    // clock start
    int start_time = clock();

    // variables
    const char *currentData = "current.csv.bz2";
    const char *knownData = "known.csv.bz2";
    std::vector<std::string> files = {"C:\\Projects\\firstUniq.csv",
                                      "C:\\Projects\\secondUniq.csv"};
    //
    const size_t itemsToReserve = 200000000;
    std::vector<std::string> file1{}; // current file from site
    file1.reserve(itemsToReserve);
    std::vector<std::string> file2{}; // latest file from archive
    file2.reserve(itemsToReserve);
    // static std::vector<std::string> pairs{}; // memory economy
    // pairs.reserve(itemsToReserve);
    static std::vector<std::string> firstUniq{}; // uniq values in file1
    firstUniq.reserve(itemsToReserve);
    static std::vector<std::string> secondUniq{}; // uniq values in file2
    secondUniq.reserve(itemsToReserve);

    // Downloading files (Todo: parallel processes)
    std::cout << "Downloading..." << std::endl;
    downloadToFile("https://xn--b1agjhrfhd.xn--b1ab2a0a.xn--b1aew.xn--p1ai/upload/expired-passports/list_of_expired_passports.csv.bz2", currentData);
    downloadToFile("http://models.tegia.ru/person_documents/list_of_expired_passports_20210906.csv.bz2", knownData);

    // Reading files
    std::cout << "Reading..." << std::endl;
    #pragma omp parallel sections
    {
        #pragma omp section
        readBZ2(currentData, file1);
        #pragma omp section
        readBZ2(knownData, file2);
    }

    // Writing first validated file into .csv
    std::cout << "Writing validated file 1..." << std::endl;
    writeToFile(file1, "list_of_expired_passports-validated.csv");

    // Parallel sorting two vectors
    std::cout << "Sorting..." << std::endl;
    std::sort(std::execution::par, file1.begin(), file1.end());
    std::sort(std::execution::par, file2.begin(), file2.end());
    std::cout << "Sorting finished" << std::endl;

    // Parallel sorting two vectors
    std::cout << "Comparison started..." << std::endl;
    compareFiles(file1, file2, firstUniq, secondUniq);
    // std::cout << "Count pairs : " << pairs.size() << std::endl;     // result : 137669478 || result on test : 7
    std::cout << "firstUniq : " << firstUniq.size() << std::endl;   // result : 4785917 || result on test : 3
    std::cout << "secondUniq : " << secondUniq.size() << std::endl; // result : 192329 || result on test : 4

    std::cout << "Writing..." << std::endl;
    writeToFile(firstUniq, "C:\\Projects\\firstUniq.csv");
    writeToFile(secondUniq, "C:\\Projects\\secondUniq.csv");

    int end_time = clock();
    int search_time = end_time - start_time;
    std::cout << "runtime = " << search_time / 1000.0 / 60.0 << std::endl; // 14.1493

    std::vector<std::string>().swap(file1);
    std::vector<std::string>().swap(file2);
    // std::vector<std::string>().swap(pairs);
    std::vector<std::string>().swap(secondUniq);
    std::vector<std::string>().swap(firstUniq);

    return 0;
}
