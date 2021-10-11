'''
To run this program, type:
python crawler.py
'''

import re
import urllib.request
from urllib.error import HTTPError,URLError
import random
from socket import timeout
import logging

start_url = "http://it.wikipedia.com"
urls = set()
explored_urls = set()
max_number_of_urls = 50
forbidden_extension = [".png", ".PNG", ".svg", ".jpg", ".JPG", ".jpeg", ".bmp", ".ico", ".gif"]
max_seconds_to_wait = 2
quiet_mode = False

links_regex = re.compile("((https?:)?\/\/(\w|\d|\.|\/|\-|\?|\=|\&|\;)*)")

def parse_links(html_code):
	links = links_regex.findall(html_code)
	links = [l[0] for l in links]

	# Removing links to images
	for l in links[:]:
		for extension in forbidden_extension:
			if extension in l:
				links.remove(l)
				break

	# Removing trailing '/'
	links_copy = links.copy()
	for l in range(0, len(links)-1):
		if(links[l].endswith("/")):
			links_copy[l] = links_copy[l][:-1]
	links = links_copy
	
	# Removing leading "//"
	links_copy = links.copy()
	for l in range(0, len(links)-1):
		if(links[l].startswith("//")):
			links_copy[l] = "http:"+links_copy[l]
		elif "//" not in links[l]:
			links_copy[l] = "http://"+links_copy[l]
	links = links_copy
	links = list(set(links))

	return links

def random_explore():
	urls.add(start_url)
	while True:
		if len(urls) == 0:
			if not quiet_mode: print("Exploration finished")
			quit()

		if len(explored_urls) >= max_number_of_urls:
			if not quiet_mode: print("Max number of explorable URLs reached.\nQuitting...")
			quit()

		# Extracting a random url
		current_url = random.choice(list(urls))
		urls.remove(current_url)

		# Downloading link data
		if not quiet_mode: print(f"Exploring \"{current_url}\"")
		try:
			response = urllib.request.urlopen(current_url, timeout=2)
		except (HTTPError, URLError) as error:
			logging.error(" Data of \"%s\" not retrieved because %s\n", current_url, error)
			continue
		except timeout:
			logging.error(" Connection timed out\n")
			continue
		except ValueError:
			logging.error(" Unknown url type\n")
			continue

		page_content = response.read().decode("utf-8")
		links = parse_links(page_content)

		for x in links:
			if x not in explored_urls:
				urls.add(x)

		if not quiet_mode: print(f"Retrieved {len(links)} link(s) ({len(urls)} waiting to be visited)\n")

		explored_urls.add(current_url)

if __name__ == "__main__":
	random_explore()