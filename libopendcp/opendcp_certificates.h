/*
    OpenDCP: Builds Digital Cinema Packages
    Copyright (c) 2010-2011 Terrence Meiczinger, All Rights Reserved

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*------------------------------------------------------------------ 
    These are OpenDCP's certificates used for digital signatures.
    They are in PEM format. You can replace this with your own.
------------------------------------------------------------------*/

/* OpenDCP ROOT certificate */
static const char *opendcp_root_cert = "-----BEGIN CERTIFICATE-----\n\
MIIESzCCAzOgAwIBAgIBBTANBgkqhkiG9w0BAQsFADB1MRQwEgYDVQQKEwtvcGVu\n\
ZGNwLm9yZzEQMA4GA1UECxMHT3BlbkRDUDEkMCIGA1UEAxMbLm9wZW5kY3AuY2Vy\n\
dGlmaWNhdGUuMi5ST09UMSUwIwYDVQQuExxVYUJmcHVOR3Z5MUsxQ3BCRzY4a0Vn\n\
SHMxaWM9MB4XDTEwMTIyMTE3MTkyNFoXDTIwMTIxODE3MTkyNFowdTEUMBIGA1UE\n\
ChMLb3BlbmRjcC5vcmcxEDAOBgNVBAsTB09wZW5EQ1AxJDAiBgNVBAMTGy5vcGVu\n\
ZGNwLmNlcnRpZmljYXRlLjIuUk9PVDElMCMGA1UELhMcVWFCZnB1Tkd2eTFLMUNw\n\
Qkc2OGtFZ0hzMWljPTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMUK\n\
Al221rbAT6x4Xek5XlpF2MdY6cQ56DTE82kiC17AHHY67sHJlT0ijK87FC36lSo4\n\
N2voY+8IxmEuXwS9zAsPXciNd2weyM7BPcy/kBDSeP0Vt4vCVKrCoLvv86aGsAxJ\n\
zshCAkxfAR7Lf0ATYMmD3JaLchVhkz0BwiuMjmuI1t1a/n5pGzmxfQkWTaTDCMgv\n\
0boas2HyXiJoU/1cTt7NpZxT4Z6SolJdLWD1HGl5IjF8FDePog/RcE4MD/rYvgdz\n\
Y+WXFe0fBRJ2zfwBTiTscaeO9q5Lr9Afb3MGMtp/UNaY4aDA7EpqLzL+jpVSYIh8\n\
J7/5XaUWUqfJVM3k8N0CAwEAAaOB5TCB4jASBgNVHRMBAf8ECDAGAQH/AgEDMAsG\n\
A1UdDwQEAwIBBjAdBgNVHQ4EFgQUUaBfpuNGvy1K1CpBG68kEgHs1icwgZ8GA1Ud\n\
IwSBlzCBlIAUUaBfpuNGvy1K1CpBG68kEgHs1ieheaR3MHUxFDASBgNVBAoTC29w\n\
ZW5kY3Aub3JnMRAwDgYDVQQLEwdPcGVuRENQMSQwIgYDVQQDExsub3BlbmRjcC5j\n\
ZXJ0aWZpY2F0ZS4yLlJPT1QxJTAjBgNVBC4THFVhQmZwdU5HdnkxSzFDcEJHNjhr\n\
RWdIczFpYz2CAQUwDQYJKoZIhvcNAQELBQADggEBAFe7Xal301yTLeTJJfqcpPNQ\n\
zCA7gVUZ18G0MEoC2cY77lKmAdwEJM6WKsh2pjM2J8aKw8TM3+Bn1OuWmzkkyzO9\n\
eVBkdUKsQw10RAx/Yheln8pOvnM3zv98t41UGCXoK7V7tXllmLnVoTiV05ILYjcl\n\
8GexuMBiVckUSIavXPkr337+1FC2DA+g3SE6Ljke5KEVe9kaf2ml4z/UsXut5rKQ\n\
PSqM1rHwX9JJU3dG/eU96kq27lKQlDZci0bgYIpls+N6neLj6JflDWf1fETtIzw6\n\
Ub5y3neQNJWGCM2ppDd2MOvTZW1fpj9Dh0J7BuOTZz0suLMOFT0m5lHQIVy56L8=\n\
-----END CERTIFICATE-----\n";

/* OpenDCP INTERMEDIATE certificate */
static const char *opendcp_ca_cert = "-----BEGIN CERTIFICATE-----\n\
MIIEUzCCAzugAwIBAgIBBjANBgkqhkiG9w0BAQsFADB1MRQwEgYDVQQKEwtvcGVu\n\
ZGNwLm9yZzEQMA4GA1UECxMHT3BlbkRDUDEkMCIGA1UEAxMbLm9wZW5kY3AuY2Vy\n\
dGlmaWNhdGUuMi5ST09UMSUwIwYDVQQuExxVYUJmcHVOR3Z5MUsxQ3BCRzY4a0Vn\n\
SHMxaWM9MB4XDTEwMTIyMTE3MTkyNVoXDTIwMTIxNzE3MTkyNVowfTEUMBIGA1UE\n\
ChMLb3BlbmRjcC5vcmcxEDAOBgNVBAsTB09wZW5EQ1AxLDAqBgNVBAMTIy5vcGVu\n\
ZGNwLmNlcnRpZmljYXRlLjEuSU5URVJNRURJQVRFMSUwIwYDVQQuExxvVGN1a1JN\n\
eEM4VnRvSUZ5Q1FPOUlwQjl0RFk9MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\n\
CgKCAQEA0hYsTKm4v5/oPrnfcEOEQnE1QC8aoT42oVsl5yNShGUJks2tiOquG2jw\n\
IvVUYYWLDdD0/Fk9WZCcZzcgNXZNGpGf1qH6xOXFxWTOWgCYk/vK/5pk+W26bOY3\n\
pJxJ4V+Aoe6QYuT6vAbswpBkLvBtw6nB8GPqdMslgTllR9QWXsqOCIPt0EQRnZe5\n\
FnvYpgfv3Wa11bygCJkevVAgTUK44nCEsufq8pasnysper+ncQFPTnol9x3xAHJw\n\
GGxzvOjZyoTWYahS5bB/VHnqsNNH6391eYMTOwt7LQ1/tvab322+pVLtAq6uiNWR\n\
KryLSdL//HU2oHRU4AtuV2uF897AlQIDAQABo4HlMIHiMBIGA1UdEwEB/wQIMAYB\n\
Af8CAQIwCwYDVR0PBAQDAgEGMB0GA1UdDgQWBBShNy6REzELxW2ggXIJA70ikH20\n\
NjCBnwYDVR0jBIGXMIGUgBRRoF+m40a/LUrUKkEbryQSAezWJ6F5pHcwdTEUMBIG\n\
A1UEChMLb3BlbmRjcC5vcmcxEDAOBgNVBAsTB09wZW5EQ1AxJDAiBgNVBAMTGy5v\n\
cGVuZGNwLmNlcnRpZmljYXRlLjIuUk9PVDElMCMGA1UELhMcVWFCZnB1Tkd2eTFL\n\
MUNwQkc2OGtFZ0hzMWljPYIBBTANBgkqhkiG9w0BAQsFAAOCAQEAboGvy5nFgLYd\n\
a3rZ0pyO6Ios/pb22Wf9E8633QNQXXeJqLE7Bg7SlQRIj6xeg4FTl1HEanKBdbpw\n\
SUfRxZlCSfnK0w03CEnlXryGFkpm3zLiSKK5csn+QrhwaRwalBrmnOtQYVE1EXgg\n\
FluGiugblc0nfdblrwntDGQFTwdEuiESzWN28b1zwIwSpzAdjth1a+gHko0icCHI\n\
bh15HnnrhV6oDl2Ji+rYNhL9bhT7lth52Fyj689s5hG7znq+2/WOotsgkCEya+0D\n\
PyGfFc9KAvzQbhQUqBqQKDN30zTobCiOV4Q/6ixzrOz3BzDAoLb5/SP+cKFgW5N6\n\
7DE9Qf5V2w==\n\
-----END CERTIFICATE-----\n";

/* OpenDCP signer certificate */
static const char *opendcp_signer_cert = "-----BEGIN CERTIFICATE-----\n\
MIIETjCCAzagAwIBAgIBBzANBgkqhkiG9w0BAQsFADB9MRQwEgYDVQQKEwtvcGVu\n\
ZGNwLm9yZzEQMA4GA1UECxMHT3BlbkRDUDEsMCoGA1UEAxMjLm9wZW5kY3AuY2Vy\n\
dGlmaWNhdGUuMS5JTlRFUk1FRElBVEUxJTAjBgNVBC4THG9UY3VrUk14QzhWdG9J\n\
RnlDUU85SXBCOXREWT0wHhcNMTAxMjIxMTcxOTI1WhcNMjAxMjE2MTcxOTI1WjB2\n\
MRQwEgYDVQQKEwtvcGVuZGNwLm9yZzEQMA4GA1UECxMHT3BlbkRDUDElMCMGA1UE\n\
AxMcQ1Mub3BlbmRjcC5jZXJ0aWZpY2F0ZS4wLkRDUDElMCMGA1UELhMcMUZQeWV2\n\
Um4zQnlYakJRTHhsVzkzQ29Wb2JzPTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC\n\
AQoCggEBAO+RNpMzof7P2ZnTy4hccfAzotkGPXCHryqiDJH0k5mG+BaEg8m+n4vt\n\
sZGdsVLSI6okKoVrViWY7ELyIcUeFXcE+ld92NBywpHr2wuQeuF2Lh1IQf8agD+m\n\
3N6pA41DqY4JnutF/Trd+YUVMSuzrKM6F9n4Y09nuhh/XXZ45r3xpE865NWDrRSL\n\
s8SbbCl4ULl9HfA4WArZiL34Rv8wD5DgCCaXg7bu8WCJczokTXjr5Owgqt9kdW1D\n\
ER1cpxuU2WV9VdFRuUXGlndQKy55PN1/TvoeDfI54sjMQCXkPt9XhsZFDMFoh25n\n\
zQmFzb4e8CkeBYn170xT4p/umFev4xcCAwEAAaOB3zCB3DAMBgNVHRMBAf8EAjAA\n\
MAsGA1UdDwQEAwIFoDAdBgNVHQ4EFgQU1FPyevRn3ByXjBQLxlW93CoVobswgZ8G\n\
A1UdIwSBlzCBlIAUoTcukRMxC8VtoIFyCQO9IpB9tDaheaR3MHUxFDASBgNVBAoT\n\
C29wZW5kY3Aub3JnMRAwDgYDVQQLEwdPcGVuRENQMSQwIgYDVQQDExsub3BlbmRj\n\
cC5jZXJ0aWZpY2F0ZS4yLlJPT1QxJTAjBgNVBC4THFVhQmZwdU5HdnkxSzFDcEJH\n\
NjhrRWdIczFpYz2CAQYwDQYJKoZIhvcNAQELBQADggEBAK5eNl6qz4Vm0GVqylK4\n\
xrPl4SPdNmhVG83vE9D38HmdNoGPBkPCXXpk6/5P8lXdsBpE1+mLdUlP1Q/2+XOT\n\
U900YeBVmqHWP1bVVnD1t+F8UY3+k3pwUDYVYkPqZSRw4hEkyM1vsxXIRD9cE+U2\n\
25PG++hcYqxr1Hy9pl5iRU0fiEJiuPWE2kFkkB9d0D07PbjThwCweS1DtAneAzdY\n\
Omfx//pTe/oQbH+wtMblGKLmdcHJ3FlXAcorLh5qrNQ4wF/6J6kHbHEp4xWkQuya\n\
B428K07+cQPHn3L/VKsB4gauNwXktkfVP0smCZjNXisIgZ8dN4C1T9iUB6OrcaNI\n\
SbA=\n\
-----END CERTIFICATE-----\n";

/* OpenDCP private key */
static const char *opendcp_private_key ="-----BEGIN RSA PRIVATE KEY-----\n\
MIIEpQIBAAKCAQEA75E2kzOh/s/ZmdPLiFxx8DOi2QY9cIevKqIMkfSTmYb4FoSD\n\
yb6fi+2xkZ2xUtIjqiQqhWtWJZjsQvIhxR4VdwT6V33Y0HLCkevbC5B64XYuHUhB\n\
/xqAP6bc3qkDjUOpjgme60X9Ot35hRUxK7OsozoX2fhjT2e6GH9ddnjmvfGkTzrk\n\
1YOtFIuzxJtsKXhQuX0d8DhYCtmIvfhG/zAPkOAIJpeDtu7xYIlzOiRNeOvk7CCq\n\
32R1bUMRHVynG5TZZX1V0VG5RcaWd1ArLnk83X9O+h4N8jniyMxAJeQ+31eGxkUM\n\
wWiHbmfNCYXNvh7wKR4FifXvTFPin+6YV6/jFwIDAQABAoIBAB7mRvjDjyBzpKhv\n\
pe/npJaLwnRllqUeCxzfm+lzd1o1C2i0HN93o9KDjQSwJz/8dLcsRQPCbXEaAVc9\n\
Ldfj4nbggH2qcL2qH6h8mFssfnz4JkiGmmSSAXq0Rga+HAQrdwIoAYRtGZVvLhDZ\n\
Q+dUHG9NPehSXlTOlzUzsFVokLJs6ZORzk+S1VP8wLbMzr3aXWRGkNtFVZluj/SJ\n\
LzbesO+ZtGS+I31Y7GO2Z9LeC3gEBnKxbK2VkngA4DJSRSFrOPJ/6ch5t7qZDnLP\n\
CJ9EckoGR9FGX7FyV6fuMiOPV6Q/AgY1iwmF2+FLZW6UbeB8OE4iNMJfpDsIsjw2\n\
DzNZP9kCgYEA/NrU8FtDvSa62JOx989qo0YdKPMO4J9K9A2BuXBFzuYbty4aESP3\n\
G/Bso492fVz7WM+JowazUDX0N1YysE/lIIy6q+aWK815bHFthxfAQfIayfHaboGo\n\
qmLzQ+fdPz8bD8FSELTnwkoDRBLu/2PiXBlwXmewKkX/DR3575N5DL0CgYEA8owR\n\
05tRFOHZzKMxLvqXb+RW7ZLyQrsr7jWIGHlyHhKYV64vPo9SReXGwUXk5C+zav0B\n\
pzyYT1sA84ttdLNEE+Rk5Hn/DUs2oXD1dxsz4/q63gX8qnBLKvFn5gz7LmYJ8PDK\n\
2W5TuVOJy0IQU4yJigayzfjcVDE3oYqMOUZOLmMCgYEA9Vq+r3BPnbZ8LfW7BlSN\n\
DFvXpcmcURgg2gpVK4SnKGme0TP59VHp0YGNWXS3LFRSTM4tpzS0QIvqKtwImY8d\n\
LWWBKZa5d02Nmk3CUwkX9KWhmv2E0CAecx9LIERYKqvobXRQVofEL4I0AxVANi9N\n\
EcNZhzGj/pEnOSoyQWffWR0CgYEA0AC1TP52s2zYhnkxNbOr6UYuEkGhxv6TNw7r\n\
bum/pvCVeyQi2gi5Kr5aC+ev2szZlhfxsgTyLaPClhntPVZ7PH/y0kmZJEJrUFn5\n\
+DH49ztPxBXoUBZLMEGks5JQWsMhJWKq43qNDHMKyagXLj9ouFj0QFV6Ri2LItsZ\n\
VAjqBTcCgYEA46BSlJ9EOdFYDtGMfNgMDnU+h/u/u7GiTUNYkpfisYawcUrm+OJx\n\
S2gP2HyvgcexJzOSD79UQdSVp9EV4bwnHcotEppJVsRWTLy9GAwcOqaQ6XxMMj5z\n\
PekJfyla4kp6IITJRo2eYXIuGviTD8JnW0PKWq77omHavPbrUFnbLO8=\n\
-----END RSA PRIVATE KEY-----\n";
