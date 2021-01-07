import requests
import os
from requests.auth import HTTPBasicAuth

auth = HTTPBasicAuth(os.environ.get('BINTRAY_API_USER'), os.environ.get('BINTRAY_API_KEY'))

print('Fetching repositories')
deleteURLs = []
repos = requests.get('https://api.bintray.com/repos/mull-project', auth=auth)
print('Fetching versions')
for repo in repos.json():
    package = requests.get('https://api.bintray.com/packages/mull-project/' + repo['name'] + '/mull-nightly/', auth=auth).json()
    for version in package['versions']:
        deleteURLs.append('https://api.bintray.com/packages/mull-project/' + repo['name'] + '/mull-nightly/versions/' + version)

print('Deleting ' + str(len(deleteURLs)) + ' nightly packages')
for url in deleteURLs:
    r = requests.delete(url, auth=auth)
    print('.', end='', flush=True)

