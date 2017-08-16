import mitmproxy
import re

reddit_patterns = [r'https?:\/\/.*reddit\.com.*',
                   r'https?:\/\/.*\.redditmedia\.com\/.*',
                   r'https?:\/\/www\.redditstatic\.com\/.*']

class TestServer:
    def request(self, flow):
        pass

    def response(self, flow):
        url = flow.request.pretty_url
        print(flow.response.headers, flow.request.pretty_url)
        for pattern in reddit_patterns:
            if re.match(pattern, url):
                flow.response.headers["Origin"] = 'reddit.com'
                flow.response.headers.set_all("Origin-Pattern", [r'https?:\/\/.*reddit\.com.*',
                   r'https?:\/\/.*\.redditmedia\.com\/.*',
                   r'https?:\/\/www\.redditstatic\.com\/.*'])
                print("matched")
                return

def start():
    return TestServer()
