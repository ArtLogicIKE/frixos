#!python3.*

"""update server.
"""
#

import argparse
import hashlib
import http.server
import json
import logging
import os
import re
import socket
import time
import urllib.parse
import threading
import sys
from datetime import datetime, timedelta

MAC_HISTORY = {}
logger = logging.getLogger(__name__)

class UpdateHttpServer:
    def __init__(self, port, bind, catalog_dir):
        self.start_time = time.time()
        self.duration_limit = 5 * 3600  # 3 hours in seconds

        def handler(*args):
            UpdateHTTPRequestHandler(catalog_dir, *args)
        httpd = http.server.HTTPServer((bind, port), handler)
        sa = httpd.socket.getsockname()
        logger.info(f'http server starting {sa[0]}:{sa[1]} {catalog_dir}')

        self.server_thread = threading.Thread(target=httpd.serve_forever)
        self.server_thread.daemon = True
        self.server_thread.start()

    def should_exit(self):
        elapsed_time = time.time() - self.start_time
        return elapsed_time >= self.duration_limit

class RobustHTTPRequestHandler(http.server.BaseHTTPRequestHandler):
    def parse_request(self):
        """
        Extend the BaseHTTPRequestHandler.parse_request to handle and escape spaces in URLs.
        """
        try:

            if len(self.raw_requestline) > 60: # /latest requests are long
                before = self.raw_requestline[:10]  # Up to the 10th byte, unchanged
                to_modify = self.raw_requestline[10:30]  # The section where spaces will be replaced
                after =self. raw_requestline[30:]  # From the 30th byte to the end, unchanged
                # Replace spaces with underscores in the specific section
                to_modify = to_modify.replace(b' ', b'_')
                # Reassemble the raw request line
                self.raw_requestline = before + to_modify + after

            super().parse_request()

        except Exception as e:
            self.send_error(400, 'Bad Request')
            logger.error(f"Exception in parsing request: {str(e)}")
            return False

        return True

class UpdateHTTPRequestHandler(RobustHTTPRequestHandler):
    def __init__(self, catalog_dir, *args):
        try:
            self.catalog_dir = catalog_dir
            super().__init__(*args)
        except Exception as e:
            logger.info('Invalid request at init, discarded')

    def do_GET(self):
        try:
            request_path = urllib.parse.urlparse(self.path)
            content=-1;
            if request_path.path == '/latest':
                args = urllib.parse.parse_qs(request_path.query)
                logger.info('request:%s', self.path)
                logger.info('args %s', args);
                # logger.info(args)
                if 'revision' in args:
                    myrev = args.get('revision')
                    myfw = args.get('fw')
                    # logger.info(myrev)
                    content = 10;
                    if 'D' in myrev:
                        content = 14
                    if ('E' in myrev or 'EF' in myrev):
                            if 'app' in args and args['app'][0] == '1280':
                                content = 37 # versions with 1280kb still work with 1.31 (37) - might not be true for future versions
                            else:
                                content = 44 # get the NEWEST version

                            # 'safe' list, don't update these ones.
                            if 'mac' in args and args.get('mac')[0] in ['D48AFCB086E0']: #, '94E686C5CB70', '4C752558A370','94E686C5BACC','1097BD2A20C0']:
                                content = 40;

                            # USE THIS IF YOU WANT SELECTIVE TESTING OF FIRMWARE
                            if 'mac' in args and args.get('mac')[0] in ['D48AFCB100B8']: #, '94E686C5CB70', '4C752558A370','94E686C5BACC','1097BD2A20C0']:
                                # logger.info('Test MAC detected: {0} {1}'.format(myfw[0], mymac))
                                content = 49; # no new version for 2.x
                    logger.info('response content: %s', content)

                    # Update MAC_HISTORY
                    now = datetime.now()
                    if 'mac' in args:
                        mac = args.get('mac')[0]
                        if mac in MAC_HISTORY:
                            MAC_HISTORY[mac]['last_seen'] = str(now)
                            if 'times_seen' in MAC_HISTORY[mac]:
                                MAC_HISTORY[mac]['times_seen'] += 1
                            else:
                                MAC_HISTORY[mac]['times_seen'] = 1

                            if not('owner') in MAC_HISTORY[mac]:
                                MAC_HISTORY[mac]['owner'] = ''

                            # update device info
                            MAC_HISTORY[mac]['version'] = args.get('version')[0];
                            MAC_HISTORY[mac]['fw'] = args.get('fw')[0];
                        else:
                            logger.info('*** New frixos seen in the wild!');
                            MAC_HISTORY[mac] = {'first_seen': str(now), 'last_seen': str(now),
                                                'host': args.get('host')[0], 'revision': args.get('revision')[0],
                                                'version': args.get('version')[0], 'fw': args.get('fw')[0],
                                                # 'flash': args.get('flash')[0], 'app': args.get('app')[0],
                                                'times_seen':1, 'owner':''}

                        # Save MAC_HISTORY to disk
                        with open('mac_history.json', 'w') as f:
                            json.dump(MAC_HISTORY, f)

                        # Count unique calls in the last 24 hours
                        unique_calls = sum(1 for v in MAC_HISTORY.values() if datetime.strptime(v['last_seen'], '%Y-%m-%d %H:%M:%S.%f') > now - timedelta(days=1))
                        unique_calls_month = sum(1 for v in MAC_HISTORY.values() if datetime.strptime(v['last_seen'], '%Y-%m-%d %H:%M:%S.%f') > now - timedelta(days=30))
                        unique_calls_all = sum(1 for v in MAC_HISTORY.values())
                        # logger.info('Received MAC %s:\n\r%s', mac, MAC_HISTORY[mac])
                        logger.info(f'Unique last 24 hours: {unique_calls} - last 30 days: {unique_calls_month} - all time: {unique_calls_all}')

                    # logger.info(content);
                    d = json.dumps(content).encode('UTF-8', 'replace')
                    logger.debug(d)
                    logger.info('');
                    self.send_response(http.HTTPStatus.OK)
                    self.send_header('Content-Type', 'application/json')
                    self.send_header('Content-Length', str(len(d)))
                    self.end_headers()
                    self.wfile.write(d)
            elif request_path.path == '/timezone':
                try:
                    args = urllib.parse.parse_qs(request_path.query)
                    logger.info('timezone request:%s', self.path)
                    logger.info('timezone args:%s', args)
                    response = "UTC"
                    self.send_response(http.HTTPStatus.OK)
                    self.send_header('Content-Type', 'text/plain')
                    self.send_header('Content-Length', str(len(response)))
                    self.send_header('Connection', 'keep-alive')  # Add keep-alive header
                    self.end_headers()
                    self.wfile.write(response.encode('UTF-8'))
                    logger.info('Sent timezone response: %s', response)
                except Exception as e:
                    logger.error('Error handling timezone request: %s', str(e))
                    self.send_error(http.HTTPStatus.INTERNAL_SERVER_ERROR, 'Internal Server Error')
            else:
                self.__send_file(self.path)
        except Exception as e:
            logger.info('Invalid request, discarded')




    def __check_header(self):
        ex_headers_templ = ['x-*-STA-MAC', 'x-*-AP-MAC', 'x-*-FREE-SPACE', 'x-*-SKETCH-SIZE', 'x-*-SKETCH-MD5', 'x-*-CHIP-SIZE', 'x-*-SDK-VERSION']
        ex_headers = []
        for ex_header in ex_headers:
            if ex_header not in self.headers:
                # logger.info('Missing header {0} to identify a legitimate request'.format(ex_header))
                return False
        return True

    def __send_file(self, path):
        if not self.__check_header():
            self.send_response(http.HTTPStatus.FORBIDDEN, 'The request available only from ESP8266 or ESP32 http updater.')
            self.end_headers()
            return

        filename = os.path.join(self.catalog_dir, path.lstrip('/'))
        # logger.debug('Request file:{0}'.format(filename))
        try:
            fsize = os.path.getsize(filename)
            self.send_response(http.HTTPStatus.OK)
            self.send_header('Content-Type', 'application/octet-stream')
            self.send_header('Content-Disposition', 'attachment; filename=' + os.path.basename(filename))
            self.send_header('Content-Length', fsize)
            self.send_header('x-MD5', get_MD5(filename))
            self.end_headers()
            f = open(filename, 'rb')
            self.wfile.write(f.read())
            f.close()
        except Exception as e:
            err = str(e)
            # logger.error(err)
            self.send_response(http.HTTPStatus.INTERNAL_SERVER_ERROR, err)
            self.end_headers()



def dir_json(path):
    d = list()
    for entry in os.listdir(path):
        e = {'name': entry}
        if os.path.isdir(entry):
            e['type'] = "directory"
        else:
            e['type'] = "file"
            if os.path.splitext(entry)[1] == '.bin':
                fn = os.path.join(path, entry)
                try:
                    f = open(fn, 'rb')
                    c = f.read(1)
                    f.close()
                except Exception as e:
                    logger.info(str(e))
                    c = b'\x00'
                if c == b'\xe9':
                    e['type'] = "bin"
                    mtime = os.path.getmtime(fn)
                    e['date'] = time.strftime('%x', time.localtime(mtime))
                    e['time'] = time.strftime('%X', time.localtime(mtime))
                    e['size'] = os.path.getsize(fn)
        d.append(e)
    return d


def get_MD5(filename):
    try:
        f = open(filename, 'rb')
        bin_file = f.read()
        f.close()
        md5 = hashlib.md5(bin_file).hexdigest()
        return md5
    except Exception as e:
        logger.error(str(e))
        return None

# Function to display version counts
def display_version_counts():
    version_counts = {}
    for mac, details in MAC_HISTORY.items():
        version = details.get('version')
        if version:
            if version in version_counts:
                version_counts[version] += 1
            else:
                version_counts[version] = 1

    logger.info(f"Version Counts all: {version_counts}")

def display_version_counts_30d():
    version_counts = {}
    now = datetime.now()
    for mac, details in MAC_HISTORY.items():
        version = details.get('version')
        lasttime = details.get('last_seen')
        if  datetime.strptime(lasttime, '%Y-%m-%d %H:%M:%S.%f') > now - timedelta(days=30):
            if version:
                if version in version_counts:
                    version_counts[version] += 1
                else:
                    version_counts[version] = 1

    logger.info(f"Version Counts 30d: {version_counts}")


def run(port=8000, bind='127.0.0.1', catalog_dir='', log_level=logging.INFO):
    global MAC_HISTORY
    logging.basicConfig( level=logging.INFO) # filename='http.log',
    logger.info(f"Frixos Web Update Server v0.30")

    if os.path.exists('mac_history.json'):
        with open('mac_history.json', 'r') as f:
            MAC_HISTORY = json.load(f)

    # Display version counts on startup
    display_version_counts()  # <- This is the new line to add
    display_version_counts_30d()

    server = UpdateHttpServer(port, bind, catalog_dir)
    while not server.should_exit():
        time.sleep(1)

    logger.info('Program has been running for the preset preiod of time. Exiting...')
    sys.exit(0)


def main():
    parser = argparse.ArgumentParser(prog='update_server', description='HTTP server for ESP8266/ESP32 update over the air.')
    parser.add_argument('catalog', type=str, nargs='?', default='', help='catalog directory with binary files')
    parser.add_argument('-b', '--bind', type=str, default='', help='bind address')
    parser.add_argument('-p', '--port', type=int, default=8000, help='listen port')
    parser.add_argument('-d', '--debug', action='store_true', help='debug mode')
    args = parser.parse_args()
    log_level = logging.DEBUG if args.debug else logging.INFO
    run(port=args.port, bind=args.bind, catalog_dir=args.catalog, log_level=log_level)


if __name__ == "__main__":
    main()
