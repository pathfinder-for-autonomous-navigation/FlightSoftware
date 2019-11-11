import yagmail
import imaplib
import base64
import os
import email


def create_txt_file(name):
    # Create a txt file
    name = name+'.txt'
    f = open(name, "w+")
    # Write whatever goes in the file here
    f.write("Hello!")
    f.close()
    return name


def sendFile(file_name):
    # Need to create an app specific password for the gmail account
    password = os.environ['PAN_PASS']
    yag = yagmail.SMTP('pan.ssds.qlocate', password)
    # Send the txt file to the designated address
    imei_num = os.environ['IMEI_NUM']
    yag.send('data@sbd.iridium.com', imei_num, file_name)


def downloadFiles():
    mail = imaplib.IMAP4_SSL("imap.gmail.com", 993)
    password = os.environ['PAN_PASS']
    mail.login('pan.ssds.qlocate@gmail.com', password)
    mail.select('Inbox')
    # .search() searches from mail. Data gives id's of all emails.
    type, data = mail.search(
        None, '(FROM "sbdservice@sbd.iridium.com")', '(UNSEEN)')
    mail_ids = data[0]
    id_list = mail_ids.split()

    found_packets = []

    for num in id_list:
        # .fetch() fetches the mail for given id where 'RFC822' is an Internet
        # Message Access Protocol.
        typ, data = mail.fetch(num, '(RFC822)')

        # go through each component of data and print sender and subject line
        for response_part in data:
            if isinstance(response_part, tuple):
                # converts message from byte literal to string removing b''
                msg = email.message_from_string(
                    response_part[1].decode('utf-8'))
                email_from = msg['from']
                email_subject = msg['subject']
                print('From : ' + email_from + '\n')
                print('Subject : ' + email_subject + '\n')

                # get the file attached to the message
                for part in msg.walk():
                    print("Fetching attachment")

                    if part.get_content_maintype() == 'multipart':
                        continue

                    if part.get('Content-Disposition') is None:
                        continue

                    # check if there is an attachment
                    if part.get_filename() is not None:
                        '''
                        fileName=part.get_filename()
                        #give a filepath for where you want to store the data
                        att_path = os.path.join('/Users/fatimayousuf/Desktop/sbdData/', fileName)
                        #if the file does not already exist, create a file for the attachment
                        if not os.path.isfile(att_path) : 
                            fp = open(att_path, 'wb')
                            fp.write(part.get_payload(decode=True))
                            fp.close()
                        #print the contents of the file
                        myfile=open(att_path, 'r')
                        '''
                        attachmentContents = part.get_payload(
                            decode=True).decode('utf8')
                        found_packets.append(attachmentContents)
                        print("Attachment Contents: " + attachmentContents)

    return found_packets

# next step is to process sbd files


if __name__ == '__main__':
    sendFile(create_txt_file("message"))
