import requests
import json
import hashlib
import time
import random
import urllib.parse
random.seed(time.time()/1e3)
# Interesting api
# api-bnolauncher.bandainamco-ol.jp
# object-bnolauncher-ct.bandainamco-ol.jp
# object-bnolauncher-pf.bandainamco-ol.jp
# object-web.blue-protocol.com
# datastore-main.aws.blue-protocol.com
# masterdata-main.aws.blue-protocol.com
# flg-main.aws.blue-protocol.com
# g-ahpatch-prod.blue-protocol.com

# deviceid combination of these  string? MD5 or MD5(SHA1)? i have no idea
# trace-config
# network-type
# product-version
# v8-version
# user-agent
# chrome-bitness
# os-name
# os-version
# os-wow64
# local
# os-session
# os-arch
# cpu-family
# cpu-model
# cpu-stepping
# num-cpus
# physical-memory
# cpu-brand
# clock-domain
# highres-ticks
# %u-%u-%u %d:%d:%d
# trace-capture-datetime

# Blue protocol launcher request step:
# POST request to 
# api-bnolauncher.bandainamco-ol.jp/api/login/url or api-bnolauncher.bandainamco-ol.jp/api/content/login/url
# with json data
# {"acc_id":"","auto_login":true,"debug_user":false,"device_id":"md5 hash?"} I'm not really sure what device id is. MD5 hash? or MD5(SHA1)?
# 
# customize_id="launcher"
# client_id="bn_ocean"
# Response:
# {
#     "error_code":0,
#     "access_url":"https://account.bandainamcoid.com/login.html?customize_id=launcher&client_id=bno_ocean&redirect_uri=https://api-bnolauncher.bandainamco-ol.jp/api/login/bana/result/<some access token>",
#     "access_token":<some access token>,
#     "cache_clear_date":"2022-12-27 10:20:20Z"
# }

# GET account-api.bandainamcoid.com/v3/login/init
# with payload content-type'application/x-www-form-urlencoded; charset=UTF-8'
# client_id	bno_ocean
# redirect_uri	https://api-bnolauncher.bandainamco-ol.jp/api/login/bana/result/<some access token>
# customize_id	launcher
# language	en
# prompt	
# sns_not_found	
# lockout_child	
# cookie	{}
# backto	
# 

# Cookies
#  Ocean hash is generated using murmurhash and bitconverter 
# this.message.Content = (HttpContent) new StringContent(Encoding.UTF8.GetString(ms.ToArray()), Encoding.UTF8, "application/json");
# if (!string.IsNullOrEmpty(ProtocolRequester._token))
# {
#     string str = "ocean_session=" + ProtocolRequester._token + ";";
#     string s = this.requestObject.ToJson() + "@@@" + ProtocolRequester._token;
#     using (MemoryStream inputStream = new MemoryStream(Encoding.UTF8.GetBytes(s)))
#     {
#     byte[] hash = MurmurHash.Create128(2929U, preference: AlgorithmPreference.X86).ComputeHash((Stream) inputStream);
#     this.message.Headers.Add("Cookie", str + "ocean_hash=" + BitConverter.ToString(hash).ToLower().Replace("-", "") + ";");
#     }
# }

# signup
# https://account-api.bandainamcoid.com/v3/signup/end

# AUTH API
# POST https://account-api.bandainamcoid.com/v3/authcode/input
# Payload
# {
#     "client_id" : "idportal",
#     "backto"    : "",
#     "customize_id" : "",
#     "authcode" : "",
#     "authcode_key" : "",
#     "language" : "en",
#     "permit_token" : "",
#     "cookie" : "",
# }

# Agree login bp
# POST https://account-api.bandainamcoid.com/v3/login/aggree
# Payload 
# {
#     "client_id" : "bno_ocean",
#     "redirect_uri"    : "https://api-bnolauncher.bandainamco-ol.jp/api/login/bana/result/gPm3gWekj8UkJKkWFOpx",
#     "customize_id" : "launcher",
#     "language" : "en",
#     "cookie" : "",
# }



USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) bandai_namco_launcher/2.0.24 Chrome/78.0.3904.113 Electron/7.1.2 Safari/537.36"
API_LAUNCHER = "https://api-bnolauncher.bandainamco-ol.jp"
API_ACCOUNT = "https://account-api.bandainamcoid.com"
DEBUG_MODE = True
CUID = "ELrmVeJRPRZMwrerwsJWkTxpKQABgNw3"


bnidApi = 'https://account-api.bandainamcoid.com/v3/login/idpw'
bpApi = 'https://blue-protocol.com/api/account/login'


def getBPLoginCode(email, password, accessToken, content=False, cuid=None):
    body = {
        'client_id': "bno_ocean",
        'redirect_uri': f"{API_LAUNCHER}/api/login/bana/result/{accessToken}",
        'customize_id': 'launcher',
        'login_id': email,
        'password': password,
        'retention': 1,
        'language': "en",
        'cookie': json.dumps({}),
        'prompt': ''
    }

    header = {'Content-Type' : 'application/x-www-form-urlencoded; charset=UTF-8',
            'User-Agent' : USER_AGENT      
          }
    
    s = requests.session()
    endpoint = "/v3/login/idpw"
    if content:
        body['client_id'] = 'bp_service'
        body['customize_id'] = ""
        body['redirect_uri'] = f"{API_LAUNCHER}/api/content/login/bana/result/{accessToken}",

    s = s.post(API_ACCOUNT+endpoint,headers=header,data=body)
    # if DEBUG_MODE:
        # print(json.dumps(s.json(),indent=4))
    response = s.json()
    redirect = response['redirect']
    cookie = {
        "loginProd" : response['cookie']['login']['value'],
        "login_common" : 1,
        "mnwlogindata" : response['cookie']['mnw']['value'],
        "__c__loginProd" : response['cookie']['login_check']['value'],
        "retention" : 1
    }
    loginCode = redirect.split('&')[-1][5:]
    return cookie, redirect, loginCode


def loginInit(cookie, accessToken):
    body = {
        'client_id': "bno_ocean",
        'redirect_uri': f"{API_LAUNCHER}/api/login/bana/result/{accessToken}",
        'customize_id': 'launcher',
        'retention': 1,
        'language': "en",
        'cookie': json.dumps(cookie),
        'prompt': '',
        'backto' : ''
    }
    print(cookie)

    header = {'Content-Type' : 'application/x-www-form-urlencoded; charset=UTF-8',
            'User-Agent' : USER_AGENT      
          }
    
    s = requests.session()
    endpoint = "/v3/login/init?"
    bodyUrlEncoded = urllib.parse.urlencode(body)

    response = s.get(API_ACCOUNT+endpoint+bodyUrlEncoded,headers=header,verify=True)
    if response.status_code==200:
        if DEBUG_MODE:
            print(json.dumps(response.json(),indent=4))
        response = response.json()
        redirect = response['redirect']
        loginCode = redirect.split('&')[-1][5:]
        return redirect, loginCode
    return response.status_code

def getBPSession(loginCode):
    header = { 'User-Agent' : USER_AGENT,
              }
    body = {
        "code": loginCode,
        "request": None}
    s = requests.session()
    s = s.post(bpApi,headers=header,data=json.dumps(body))
    return s.headers


def getAccessToken(accId=None, content=False, cuId=None):
    deviceid = "8a06386b222e461dbac09934fec2527e" # Having a valid deviceid is a must, currently i have no idea, what did bandai hash to get current value
    header = { 'User-Agent' : USER_AGENT,
                'Content-Type' : 'application/json; charset=utf-8' 
              }
    body={
        "acc_id" : "",
        "auto_login": True,
        "debug_user": False,
        "device_id" : deviceid
    }
    endpoint = '/api/login/url'
    if content:
        body['cuid'] = cuId
        endpoint = '/api/content/login/url'
        body["acc_id"] = accId
    s = requests.session()
    s = s.post(API_LAUNCHER+endpoint, json=body)
    response = s.json()
    accessToken = response['access_token']
    if DEBUG_MODE:
        print(json.dumps(response, indent=4))
    return accessToken

def getAccountId(cookie, redirect, accessToken, loginCode):
    header = { 'User-Agent' : USER_AGENT,
                'Content-Type' : 'application/x-www-form-urlencoded; charset=UTF-8'
              }
    body={
        "acc_id" : "",
        "access_token" : accessToken
    }
    endpoint = '/api/login/bana/result/' + accessToken
    s = requests.session()
    response = s.get(API_LAUNCHER+endpoint,data={'code':loginCode})
    if DEBUG_MODE:
        print("Get Account ID")
        print(response.content)
        
    if response.status_code==200:
        header['Content-Type'] = 'application/json; charset=utf-8'
        endpoint = '/api/login/state'
        response = s.post(API_LAUNCHER + endpoint, json=body)
        if DEBUG_MODE:
            print('Get Login State - AccountID')
            print(json.dumps(response.json(),indent=4))
        err = response.json()['error_code']
        loginState = response.json()['login_state']
        accountId = response.json()['acc_id']
        return err, loginState, accountId
    

def getCuId(accountId):
    header = { 'User-Agent' : USER_AGENT,
            'Content-Type' : 'application/json; charset=utf-8' 
            }
    body = {"acc_id":accountId,
            "content_group_key":"",
            "language":0,
            "releasenotes":True}
    s = requests.session()
    endpoint = '/api/content/list'
    response = s.post(API_LAUNCHER+endpoint,json=body)
    if DEBUG_MODE:
        print(json.dumps(response.json(),indent=4))
    cuId = response.json()['pc_content'][1]['cuid']
    return cuId


def getGameCode(accessToken, loginCode):
    header = { 'User-Agent' : USER_AGENT,
                'Content-Type' : 'application/x-www-form-urlencoded; charset=UTF-8'
              }
    body={
        "acc_id" : "",
        "access_token" : accessToken
    }
    endpoint = '/api/login/content/bana/result/' + accessToken
    s = requests.session()
    response = s.post(API_LAUNCHER+endpoint,data={'code':loginCode})
    if DEBUG_MODE:
        print(json.dumps(response.json(),indent=4))
    if response.status_code==200:
        header['Content-Type'] = 'application/json; charset=utf-8'
        endpoint = '/api/content/login/state'
        response = s.post(API_LAUNCHER + endpoint, json=body)
        if DEBUG_MODE:
            print(json.dumps(response.json(),indent=4))
        err = response.json()['error_code']
        loginState = response.json()['login_state']
        code = response.json()['code']
        return err, loginState, code

if __name__ == '__main__':

    launcherAccessToken = getAccessToken()

    email = 'farhannz@hotmail.com'
    pw = 'Ulahk1tu!@#'

    # simulating launcher login
    cookie, _, _ = getBPLoginCode(email, pw, launcherAccessToken)
    redirect, loginCode = loginInit(cookie, launcherAccessToken)
    err, loginState, accountId = getAccountId(cookie, redirect,launcherAccessToken, loginCode)
    cuid = getCuId(accountId)

    # simulating game start login
    gameAccessToken = getAccessToken(accId=accountId,content=True,cuid=cuid)
    err, gameLoginState, gameCode = getGameCode(gameAccessToken, loginCode)
    # print(getBPSession(loginCode))