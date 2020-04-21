package main

import (
	"crypto/rand"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"os"
	"os/user"
	"path/filepath"

	"golang.org/x/net/context"
	"golang.org/x/oauth2"
	"golang.org/x/oauth2/google"
	"google.golang.org/api/gmail/v1"
)

// NOTE : we don't want to visit CSRF URL to get the authorization code
// and paste into the terminal each time we want to send an email
// therefore we will retrieve a token for our client, save the token into a file
// you will be prompted to visit a link in your browser for authorization code only ONCE
// and subsequent execution of the program will not prompt you for authorization code again
// until the token expires.

func usage() {
    fmt.Println("./send_upink mymessage.sbd [IMEI number]")
    os.Exit(-1)
}

// getClient uses a Context and Config to retrieve a Token
// then generate a Client. It returns the generated Client.
func getClient(ctx context.Context, config *oauth2.Config) *http.Client {
	cacheFile, err := tokenCacheFile()
	if err != nil {
		log.Fatalf("Unable to get path to cached credential file. %v", err)
	}
	tok, err := tokenFromFile(cacheFile)
	if err != nil {
		tok = getTokenFromWeb(config)
		saveToken(cacheFile, tok)
	}
	return config.Client(ctx, tok)
}

// getTokenFromWeb uses Config to request a Token.
// It returns the retrieved Token.
func getTokenFromWeb(config *oauth2.Config) *oauth2.Token {
	authURL := config.AuthCodeURL("state-token", oauth2.AccessTypeOffline)
	fmt.Printf("Go to the following link in your browser then type the "+
		"authorization code: \n%v\n", authURL)

	var code string
	if _, err := fmt.Scan(&code); err != nil {
		log.Fatalf("Unable to read authorization code %v", err)
	}

	tok, err := config.Exchange(oauth2.NoContext, code)
	if err != nil {
		log.Fatalf("Unable to retrieve token from web %v", err)
	}
	return tok
}

// tokenCacheFile generates credential file path/filename.
// It returns the generated credential path/filename.
func tokenCacheFile() (string, error) {
	usr, err := user.Current()
	if err != nil {
		return "", err
	}
	tokenCacheDir := filepath.Join(usr.HomeDir, ".credentials")
	os.MkdirAll(tokenCacheDir, 0700)
	return filepath.Join(tokenCacheDir,
		url.QueryEscape("gmail-go-sendemail.json")), err
}

// tokenFromFile retrieves a Token from a given file path.
// It returns the retrieved Token and any read error encountered.
func tokenFromFile(file string) (*oauth2.Token, error) {
	f, err := os.Open(file)
	if err != nil {
		return nil, err
	}
	t := &oauth2.Token{}
	err = json.NewDecoder(f).Decode(t)
	defer f.Close()
	return t, err
}

// saveToken uses a file path to create a file and store the
// token in it.
func saveToken(file string, token *oauth2.Token) {
	fmt.Printf("Saving credential file to: %s\n", file)
	f, err := os.Create(file)
	if err != nil {
		log.Fatalf("Unable to cache oauth token: %v", err)
	}
	defer f.Close()
	json.NewEncoder(f).Encode(token)
}

func sendMessage(service *gmail.Service, userID string, message gmail.Message) {
	_, err := service.Users.Messages.Send(userID, &message).Do()
	if err != nil {
		log.Fatalf("Unable to send message: %v", err)
	} else {
		log.Println("Email message sent!")
	}

}

func createMessage(from string, to string, subject string, content string) gmail.Message {

	var message gmail.Message

	messageBody := []byte("From: " + from + "\r\n" +
		"To: " + to + "\r\n" +
		"Subject: " + subject + "\r\n\r\n" +
		content)

	// see https://godoc.org/google.golang.org/api/gmail/v1#Message on .Raw
	message.Raw = base64.StdEncoding.EncodeToString(messageBody)

	return message
}

func chunkSplit(body string, limit int, end string) string {

	var charSlice []rune

	// push characters to slice
	for _, char := range body {
		charSlice = append(charSlice, char)
	}

	var result string = ""

	for len(charSlice) >= 1 {
		// convert slice/array back to string
		// but insert end at specified limit

		result = result + string(charSlice[:limit]) + end
		// discard the elements that were copied over to result
		charSlice = charSlice[limit:]

		// change the limit
		// to cater for the last few words in
		//
		if len(charSlice) < limit {
			limit = len(charSlice)
		}

	}

	return result

}

func randStr(strSize int, randType string) string {

	var dictionary string

	if randType == "alphanum" {
		dictionary = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	}

	if randType == "alpha" {
		dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	}

	if randType == "number" {
		dictionary = "0123456789"
	}

	var bytes = make([]byte, strSize)
	rand.Read(bytes)
	for k, v := range bytes {
		bytes[k] = dictionary[v%byte(len(dictionary))]
	}
	return string(bytes)
}

func createMessageWithAttachment(from string, to string, subject string, content string, fileDir string, fileName string) gmail.Message {

	var message gmail.Message

	// read file for attachment purpose
	// ported from https://developers.google.com/gmail/api/sendEmail.py

	fileBytes, err := ioutil.ReadFile(fileDir + fileName)
	if err != nil {
		log.Fatalf("Unable to read file for attachment: %v", err)
	}

	// fileMIMEType := http.DetectContentType(fileBytes)

	// https://www.socketloop.com/tutorials/golang-encode-image-to-base64-example
	fileData := base64.StdEncoding.EncodeToString(fileBytes)

	boundary := randStr(32, "alphanum")

	messageBody := []byte("Content-Type: multipart/mixed; boundary=" + boundary + " \n" +
		"MIME-Version: 1.0\n" +
		"To: " + to + "\n" +
		"From: " + from + "\n" +
		"subject: " + subject + "\n\n" +

		"--" + boundary + "\n" +
		"Content-Type: multipart/alternative; boundary=" + boundary + " \n" + "\n\n" +

		"--" + boundary + "\n" +

		"Content-Type:  text/html; charset=" + string('"') + "UTF-8" + string('"') + "\n" +
		content + "\n\n" +

		"--" + boundary + "\n" +

		"Content-Type: application/octet-stream; name=" + string('"') + fileName + string('"') + " \n" +
		"MIME-Version: 1.0\n" +
		"Content-Transfer-Encoding: base64\n" +
		"Content-Disposition: attachment; filename=" + string('"') + fileName + string('"') + " \n\n" +
		fileData + "\n" +
		//	chunkSplit(fileData, 76, "\n") +
		"--" + boundary + "--")

	// see https://godoc.org/google.golang.org/api/gmail/v1#Message on .Raw
	// use URLEncoding here !! StdEncoding will be rejected by Google API

	message.Raw = base64.URLEncoding.EncodeToString(messageBody)

	return message
}

func main() {

    if len(os.Args) != 3 {
        usage()
    }

    imei := os.Args[2]

	ctx := context.Background()

	// process the credential file
	credential, err := ioutil.ReadFile("configs/credentials.json")
	if err != nil {
		log.Fatalf("Unable to read client secret file: %v", err)
	}

	// Use GmailSendScope for this example.
	// See the rest at https://godoc.org/google.golang.org/api/gmail/v1#pkg-constants

	config, err := google.ConfigFromJSON(credential, gmail.GmailSendScope)
	if err != nil {
		log.Fatalf("Unable to parse client secret file to config: %v", err)
	}

	client := getClient(ctx, config)

	// initiate a new gmail client service
	gmailClientService, err := gmail.New(client)
	if err != nil {
		log.Fatalf("Unable to initiate new gmail client: %v", err)
	}

	// create message without attachment
	msgContent := `<div dir="ltr"><br></div>`

	//message := createMessage("from@gmail.com", "to@gmail.com", "Email from GMail API", msgContent)

	// send out our message
	//user := "me"
	//sendMessage(gmailClientService, user, message)

	messageWithAttachment := createMessageWithAttachment("pan.ssds.qlocate@gmail.com", "data@sbd.iridium.com", imei, msgContent, "./", os.Args[1])

	// send out our message
	user := "pan.ssds.qlocate@gmail.com"
	sendMessage(gmailClientService, user, messageWithAttachment)

}
