#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <unistd.h>

#define CHUNK_SIZE (1024 * 1024)  // 1 MB chunk size

// Structure to hold the file descriptor
struct WriteThis {
    int fd;
    size_t total_written;
    size_t content_length;
};

// Callback function to write data to disk
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    struct WriteThis *out = (struct WriteThis *)stream;
    size_t written = write(out->fd, ptr, size * nmemb);

    if (written == (size * nmemb)) {
        out->total_written += written;
        if (out->content_length > 0) {
            float progress = (out->total_written / (float)out->content_length) * 100;
            printf("\rProgress: %.2f%%", progress);
            fflush(stdout);
        }
    } else {
        fprintf(stderr, "\nWrite error occurred\n");
        return 0;
    }

    return written;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <url> <output device>\n", argv[0]);
        return 1;
    }

    const char *url = argv[1];
    const char *output_device = argv[2];
    CURL *curl;
    CURLcode res;

    // Open the output device
    int fd = open(output_device, O_WRONLY);
    if (fd < 0) {
        perror("Error opening output device");
        return 1;
    }

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error initializing CURL\n");
        close(fd);
        return 1;
    }

    struct WriteThis out = {fd, 0, 0};

    // Set the URL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  // Follow redirects

    // Get the content length for progress calculation before starting download
    double content_length = 0;
    res = curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);  // Send a HEAD request
    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
        if (res == CURLE_OK && content_length > 0) {
            out.content_length = (size_t)content_length;
        } else {
            printf("Content length unavailable, proceeding without progress display.\n");
        }
    }

    // Set it back to normal GET request
    curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);

    // Perform the actual download
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
    } else {
        printf("\nDownload and write completed.\n");
    }

    // Cleanup
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    close(fd);

    return 0;
}
