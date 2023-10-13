#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <curl/curl.h>
#include <regex.h>

#define file_name "test.txt"
#define link_file  "links.csv"
#define text_file "text.csv"
#define MAX_URL_LENGTH 2048

int checkHostnameOrIp(char *info)
{
    if(info[1] > 47 && info[1] < 58)
        return 1; //is IP
    else
        return 0; // is Hostname
}

void get_ip(char * hostname) 
{
    struct hostent *host_info = gethostbyname(hostname);      
    if(host_info != NULL){
        int i = 0;
        while(host_info->h_addr_list[i] != NULL){
            struct in_addr *official = (struct in_addr*) host_info->h_addr_list[i];
            if(i == 0){
                printf("Official IP: %s\n", inet_ntoa(*official));
            }else{
                printf("Alias IP: \n");
                printf("%s\n", inet_ntoa(*official));
            }
            i++;
        }
    }
    else{
        printf("Not found information!\n");
        return;
    }
}

void get_hostname(char *ip)
{
    struct in_addr addr;
    if(inet_pton(AF_INET, ip, &addr) == 1){
        struct hostent* host_info = gethostbyaddr(&addr, sizeof(struct in_addr), AF_INET);
        if(host_info != NULL){
            printf("Official name: %s\n",host_info->h_name);
            char **alias = host_info->h_aliases;
            if(*alias != NULL){
               printf("Alias name: \n");
                while(*alias != NULL){
                    printf("%s\n",*alias);
                    alias++;
                } 
            }
        }
        else{
            printf("Not found information!\n");
            return;
        }
    }else {
        printf("Not found information!\n");
        return;
    }
    
}

void write2File(const char* url){
    CURL *curlHandle = curl_easy_init();
    curl_easy_setopt(curlHandle, CURLOPT_URL, url);
    FILE *file = fopen(file_name, "w+");
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, file);
    curl_easy_perform(curlHandle);                        
    fclose(file);
    curl_easy_cleanup(curlHandle);
}

char* getFullUrl(const char* url){
    char* fullUrl = malloc(strlen(url) + strlen("https://") + 1);
    strcpy(fullUrl, "https://");
    strcat(fullUrl, url);
    return fullUrl;
}

void extract_hyperlinks(const char *html_file_name, char ***links,char*** text, int *num_texts, int *num_links)
{
    char *html = NULL;
    size_t html_size = 0;
    FILE *file = fopen(html_file_name, "r");
    if (file == NULL)
    {
        perror("Error opening HTML file");
        return;
    }
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file))
    {
        size_t len = strlen(buffer);
        char *new_html = realloc(html, html_size + len + 1);
        if (new_html == NULL)
        {
            perror("Memory allocation error");
            free(html);
            fclose(file);
            return;
        }
        html = new_html;
        strcpy(html + html_size, buffer);
        html_size += len;
    }
    fclose(file);
    regex_t hl_regex_links;
    regex_t hl_regex_text;
    regcomp(&hl_regex_links, "<a\\s+href=\"([^\"]+)\"[^>]*>", REG_ICASE | REG_EXTENDED);
    regcomp(&hl_regex_text, "<p>.*<a[^>]*>([^<]+)</a>.*</p>", REG_ICASE | REG_EXTENDED);
    regmatch_t pmatch[2];
    *num_links = 0;
    char **link_list = NULL;
    const char *cursor = html;
    while (regexec(&hl_regex_links, cursor, 2, pmatch, 0) == 0)
    {
        int start = pmatch[1].rm_so;
        int end = pmatch[1].rm_eo;
        char *link = malloc(end - start + 1);
        strncpy(link, cursor + start, end - start);
        link[end - start] = '\0';
        char **new_link_list = realloc(link_list, (*num_links + 1) * sizeof(char *));
        if (new_link_list == NULL)
        {
            perror("Memory allocation error");
            for (int i = 0; i < *num_links; i++)
            {
                free(link_list[i]);
            }
            free(link_list);
            free(html);
            regfree(&hl_regex_links);
            return;
        }
        link_list = new_link_list;
        link_list[(*num_links)++] = link;
        cursor += end;
    }
    cursor = html;
    char** text_list = NULL;
    *num_texts = 0;
    while (regexec(&hl_regex_text, cursor, 2, pmatch, 0) == 0)
    {
        int start = pmatch[1].rm_so;
        int end = pmatch[1].rm_eo;
        char *text = malloc(end - start + 1);
        strncpy(text, cursor + start, end - start);
        text[end - start] = '\0';
        char **new_text_list = realloc(text_list, (*num_texts + 1) * sizeof(char *));
        if (new_text_list == NULL)
        {
            perror("Memory allocation error");
            for (int i = 0; i < *num_texts; i++)
            {
                free(text_list[i]);
            }
            free(text_list);
            free(html);
            regfree(&hl_regex_text);
            return;
        }
        text_list = new_text_list;
        text_list[(*num_texts)++] = text;
        cursor += end;
    }
    *text = text_list;
    *links = link_list;
    free(html);
    regfree(&hl_regex_links);
}
int main(int argc, char *argv[])
{
    if(argc <2){
        printf("Usage: %s <url>\n", argv[0]);
        return 1;
    }
    char *fullUrl = getFullUrl(argv[1]);
    write2File(fullUrl);
    char **final_links = NULL;
    int final_count = 0;
    char **final_texts = NULL;
    int final_count_text = 0;
    extract_hyperlinks(file_name, &final_links, &final_texts,&final_count_text, &final_count);
    //write to file
    FILE *file = fopen(link_file, "w+");
    for (int i = 0; i < final_count; i++)
    {
        fprintf(file, "%s\n", final_links[i]);
    }
    fclose(file);
    file = fopen(text_file, "w+");
    for (int i = 0; i < final_count_text; i++)
    {
        fprintf(file, "%s\n", final_texts[i]);
    }
    fclose(file);
    return 0;
}