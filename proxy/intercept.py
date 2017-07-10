# import libmproxy
import mitmproxy
import subprocess
import threading

from collections import defaultdict
from mitmproxy import ctx
from mitmproxy.script import concurrent
import tld


rules = defaultdict(dict)


prelist = {
    'sb.scorecardresearch.com': '0',
    'www.google-analytics.com': '0',
    '*.doubleclick.net': '0',
    'ajax.googleapis.com': '1',
    '*.bootstrapcdn.com': '1',
}


def _is_host_approved(address, host):
    while True:
        if host in prelist:
            return prelist[host] == '1'
        if host in rules[address]:
            return rules[address][host] == '1'
        try:
            host = host[host.index('.') + 1:]
            if ('*.' + host) in prelist:
                return prelist['*.' + host] == '1'
            if ('*.' + host) in rules[address]:
                return rules[address]['*.' + host] == '1'
        except ValueError:
            return None


# @concurrent
def request(flow):
    global r
    try:
        tld.get_tld(flow.request.host_header, fix_protocol=True)
    except tld.exceptions.TldDomainNotFound as e:
        flow.kill()
        return

    # ctx.log.error(str(rules))
    approved = _is_host_approved(flow.client_conn.address.host,
                                 flow.request.host_header)
    if approved is None:
        
        rlock.acquire()
        r = flow.request.url
        approved = _is_host_approved(flow.client_conn.address.host, flow.request.host_header)
        if approved is None:
            rules[flow.client_conn.address.host][flow.request.host_header] = '0'
            p = subprocess.getoutput('echo ' + flow.client_conn.address.host +
                                     ' ' + flow.request.host_header +
                                     '| /bin/qrexec-client-vm dom0 '
                                     'qubes.SecureWorkstationNetfilter')
            rules[flow.client_conn.address.host][flow.request.host_header] = \
                p.strip()
            approved = \
                rules[flow.client_conn.address.host][flow.request.host_header] == \
                '1'

    if not approved:
        flow.response = mitmproxy.http.HTTPResponse.make(403)
    print('endrequest')

r = None
rlock = threading.Lock()
def responseheaders(flow):
    global r
    if flow.server_conn.ssl_established:
        ctx.log.error(flow.request.host_header + ' ' + str('alts:' + str(flow.server_conn.cert.altnames)))
        for name in flow.server_conn.cert.altnames:
            name = str(name, 'utf-8')
            rules[flow.client_conn.address.host][name] = '1'
    if r == flow.request.url:
        r = None
        rlock.release()
        # print(flow.server_conn.cert.altnames, flow.server_conn.cert.cn)

    # print(flow.client_conn.address.host)
    # print(flow.request.host, flow.request.host_header)

def error(flow):
    global r
    if r == flow.request.url:
        r = None
        rlock.release()
    return
