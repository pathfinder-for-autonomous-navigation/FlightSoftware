#!/bin/bash

python3.7 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
matlab -batch '"'"pyversion $(pwd)/venv/bin/python"'"' -nodisplay -nojvm -nosplash
export PYTHONPATH=$(pwd)/venv
if [ -d "$HOME/MATLAB/R2019b/extern/engines/python" ]
then
	cd $HOME/MATLAB/R2019b/extern/engines/python
else
	cd /Applications/MATLAB_R2019b.app/extern/engines/python
fi
python setup.py install
cd -
git submodule update --init --recursive
pio run -e fsw_native_leader
