# get operating system to determine location of elasticsearch.yml file
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    *)          machine="UNKNOWN:${unameOut}"
esac

#set location of elasticsearch.yml file base on system operating system
if [ "$machine" == "Linux" ]; then
  echo "Configuring elasticsearch.yml on Linux at:"
  file="/etc/elasticsearch/elasticsearch.yml" 
elif [ "$machine" == "Mac" ]; then
  echo "Configuring elasticsearch.yml on Mac at:"
  file="/usr/local/etc/elasticsearch/elasticsearch.yml"
elif [ "$machine" == "Cygwin" ]; then
  echo "Configuring elasticsearch.yml on Windows at:"
  file="C:\ProgramData\Elastic\Elasticsearch\config\elasticsearch.yml"
else
  echo "Unable to Configure elasticsearch.yml"
  exit 1
fi

echo "$file"

# if first config is not in file already, write it to file
if sudo grep -q 'http.cors.enabled: true' $file; then
  echo "elasticsearch.yml was already partially configured..."
else
  sudo /bin/su -c "echo 'http.cors.enabled: true' >> $file"
fi

# if second config is not in file already, write it to file
if sudo grep -q "http.cors.allow-origin:" $file; then
  echo "elasticsearch.yml was already partially configured..."
else
  sudo /bin/su -c "echo 'http.cors.allow-origin: /https?:\/\/localhost(:[0-9]+)?/' >> $file"
fi

echo "Configuration Complete at $file"