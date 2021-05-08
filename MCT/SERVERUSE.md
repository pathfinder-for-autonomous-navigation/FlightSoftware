### First Time Use:

1. create or acquire  a public ssh key to be given to someone who already has access to server. This is needed to allow you to connect with the server.

### Every Time Use:

1. ssh into the server using: `ssh masterchief@<server-ip>`
2. in another terminal on your local machine create an ssh tunnel using: `ssh masterchief@<server-ip> -NL 9200:localhost:9200`
3. start elasticsearch using: `sudo systemctl start elasticsearch`
4. start TLM using `python -m tlm`
5. start MCT by navigating to MCT directory and running `npm start ptest/config/CONFIG_NAME.json`
6. Now you can start a local ptest case and view the data on the server's ground software at `https://<server-ip>:8080` - make sure the config file imei numbers you use in the local ptest case match the imei numbers in the devices list in the `CONFIG_NAME.json` file
