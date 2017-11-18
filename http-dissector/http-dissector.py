from scapy.all import *
import hashlib
import datetime
import os.path
import sys


class HTTP_Dissector:
    '''
     example_flow = {
        "end-points": {
            "host": "<IP_HOST>:<PORT_HOST>",
            "server": "<IP_SERVER>:<PORT_SERVER>"
                },
        "method": "<HTTP-METHOD>",
        "content": {
            "name": "<CONTENT_NAME>",
            "bytes": b"<FILE_BYTES>"
            }
        "headers": {
            "host": "<REQUEST_HEADER>",
            "server": "<RESPONSE_HEADER>"
            }
        }
    '''

    def __init__(self, save_dir="", log=True, test=False):
        self.flows = dict()
        self.log = ""
        self.save_log = log
        if save_dir != "" and save_dir[-1] != "/":
            self.save_dir = save_dir + "/"
        else:
            self.save_dir = save_dir

        if test:
            self.flows["0.0.0.0:99 <> 1.1.1.1:80"] = HTTP_Dissector.new_flow()
            self.add_log( "Adding dummy flow")


    def get_flow_name(self, session):
        '''
            session = "TCP 192.168.115.238:1127 > 66.7.200.69:80"
            split: ['TCP', '192.168.115.238:1127', '>', '66.7.200.69:80']

                                    HOST <> SERVER
            return '192.168.115.238:1127 <> 66.7.200.69:80'
        '''

        parts = session.split()
        if parts[3][-2:] == '80':
            server = parts[3]
            host = parts[1]
            self.add_log("Session: HOST({}) -> SERVER({})".format( host, server))
        else:
            server = parts[1]
            host = parts[3]
            self.add_log("Session: SERVER({}) -> HOST({})".format( server, host))

        return "{} <> {}".format(host, server)


    def new_flow():
        flow = {
            "end-points": {
                "host": None,
                "server": None
                    },
            "method": None,
            "content": {
                "host": {
                    "name": None,
                    "bytes": None,
                    "sha256": None
                    },
                "server": {
                    "name": None,
                    "bytes": None,
                    "sha256": None
                    },
                },
            "headers": {
                "host": {
                    "<REQUEST_HEADER_ITEM>": None
                    },
                "server": {
                    "Content-Type": None
                    }
                }
            }

        return flow


    def add_log(self, text):
        self.log += str(datetime.datetime.now()) + ": " + text + "\n"


    def get_content(self, payload):
        header_end = payload.find(b"\r\n\r\n")
    
        if header_end is not -1:
            header = dict()
            transferred_file = None
            try:
                #header_string = str(payload[: header_end + 2]) #  With '\\r\\n'
                #header = dict( re.findall( r'\\n(?P<name>[\w-]*?): (?P<value>.*?)\\r', header_string ) )
                header_string = payload[: header_end].decode()
                header_split = header_string.split('\r\n')
                header["HTTP"] = header_split[0]
                self.add_log("\tUnpacking: {}".format(header_split[0]))

                for i in header_split[1:]:
                    if ":" in i:
                        self.add_log("\tUnpacking: {}".format(i))
                        [k, v] = i.split(":", 1)
                        if k and v:
                            header[k] = v
                        else:
                            print("[!] Unknow content: {}".format(i))
                            self.add_log("[!] Failed to split '{}'".format(i))

                    else:
                        print("[!] Unknow content: {}".format(i))
                        self.add_log("[!] Failed to split '{}'\n".format(i))

                if "Content-Type" in header.keys():
                    self.add_log("Content found")
                    transferred_file = payload[header_end + 4: ]    #  After '\\r\\n\\r\\n'
    

                if "User-Agent" in header.keys():
                    print("[!] Host Header: {}".format(header["HTTP"]))
                    encaps_header = {"host": header}
                elif "HTTP" in header["HTTP"]:
                    print("[!] Server Header: {} \tT_File: {}".format(header["HTTP"], len(transferred_file)))
                    encaps_header = {"server": header}
                else:
                    print("[!] Header do not contains 'Content-Type' either 'User-Agent'\n\t{}".format(str(header)))
                    return None, None

                return encaps_header, transferred_file
            except Exception as e:
                msg = "[!!] Error in 'get_content()'\n\t{}".format(e)
                print(msg)
                self.add_log( msg )
    
        return None, None
    
    
    def save_data(self):
#        cont = 0
        for k, flow in self.flows.items():
            #if "Content-Enconding" in flow["headers"]["server"].keys():
            #    pass
            
            #if "image" in flow["headers"]["server"]["Content-Type"]:
            #    pass
            
        
            try:
                for end_point, content in flow["content"].items():
                    if content["bytes"] is not None:
                        file_name = content["name"]
                        content_bytes = content["bytes"]    # TODO: unzip

                        sha256 = hashlib.sha256()
                        sha256.update(content_bytes)
                        content["sha256"] = sha256.hexdigest()
                        
                        file_full_path = self.save_dir + file_name
                        # Duplicates
                        file_try = file_full_path
                        cnt = 1
                        while os.path.exists(file_try):
                            self.add_log("! '{}' already exists.".format(file_try))

                            file_try = "{}({})".format(file_full_path, cnt)
                            cnt += 1

                        file_full_path = file_try

                        # Save

                        with open(file_full_path, "w+b") as f:
                            f.write(content["bytes"])
                        self.add_log( "! '{}' succefull extracted.".format(file_full_name) )

            except Exception as e:
                self.add_log( "[!!] Failed to save file from '{}'\n\t".format(k, e))

        if self.save_log:
            with open("log.txt", "a+") as log:
                log.write(self.log)
    
    
    
    def parser_http(self, in_pcap):
        pcap = rdpcap(in_pcap)
        sessions = pcap.sessions()
    
        for s, packets in sessions.items():
            print("Session: {s}".format(s=s))
            session_payload = b''
            is_http = True
   
            for packet in packets:
                # Only TCP packets
                if TCP not in packet:
                    is_http =  False
                    break
                    
                # Only http protocol sessions
                if packet[TCP].sport != 80 and packet[TCP].dport != 80:
                    is_http =  False
                    break

                session_payload += bytes(packet[TCP].payload)

            if not is_http:
                continue

            flow_name = self.get_flow_name(s)
            enc_header, t_file = self.get_content(session_payload)
            if flow_name not in self.flows.keys():
                self.flows[flow_name] = HTTP_Dissector.new_flow()
                self.add_log( "! New flow: '{}'".format(flow_name))

            if enc_header is not None:
                if "host" in enc_header.keys():
                    # Host -> Server
                    header = enc_header["host"]

                    http_splitted = header["HTTP"].split()
                    method = http_splitted[0]
                    file_name = http_splitted[1].split("/")[-1]

                    self.flows[flow_name]["end-points"]["host"] = flow_name.split()[0]
                    self.flows[flow_name]["method"] = method
                    self.flows[flow_name]["headers"]["host"] = header
                    self.flows[flow_name]["content"]["server"]["name"] = file_name

                    if t_file is not None:
                        self.flows[flow_name]["content"]["host"]["name"] = "{}_req".format(file_name)
                        self.flows[flow_name]["content"]["host"]["bytes"] = t_file

                elif "server" in  enc_header.keys():
                    # Server -> Host
                    header = enc_header["server"]

                    self.flows[flow_name]["end-points"]["server"] = flow_name.split()[2]
                    self.flows[flow_name]["headers"]["server"] = header
    
                    if t_file is not None:
                        self.flows[flow_name]["content"]["server"]["bytes"] = t_file

                    print("########## Packet HTTP header ##########")
                    print("Packet content type: {}".format(header["Content-Type"]) )
                    print("Transfered file: len: {}".format(len(t_file) ) )
                    print("########## ######### ######## ##########\n")
                    #save_data(header, t_file)
            else:
                print(": No header\n")
                self.add_log("[!] No header")
    
        self.save_data()
    

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage:\n\tpython3 http-dissector.py file.pcap")
    else:
        dissector = HTTP_Dissector()
        dissector.parser_http(sys.argv[1])

