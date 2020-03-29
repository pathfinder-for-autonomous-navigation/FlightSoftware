import pty, subprocess, os, serial, pathlib, shutil
from ..state_session import StateSession

def setup_mock():
    master_fd, slave_fd = pty.openpty()
    test_path = os.path.dirname(os.path.realpath(__file__))
    fswmock_path = os.path.join(test_path, "fswmock.py")
    binary_process = subprocess.Popen(["python", fswmock_path], stdout=master_fd, stderr=master_fd, stdin=master_fd)

    logs_path = os.path.join(test_path, "logs")
    shutil.rmtree(logs_path)
    os.mkdir(logs_path)
    state_session = StateSession("fswmock", logs_path)
    state_session.connect(os.ttyname(slave_fd), 9600)
    return binary_process, state_session

def test_read_state():
    p, state_session = setup_mock()
    assert state_session.read_state("foo") == "0"
    assert state_session.read_state("bar") == "false"
    state_session.disconnect()
    p.terminate()

test_read_state()