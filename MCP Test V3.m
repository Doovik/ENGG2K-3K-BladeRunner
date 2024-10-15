
% Simple MCP for MVP testing of 2024 ENGG2K3K Bladerunners
%
%   Accepts connection requests and can send FSLOWC, FFASTC, STOPC, STOPO actions,
%   as well as OPEN and CLOSE door commands.
% 
% Author: Alan
% Date: 9 September 2024
% Revision History:
%   V1.1: Updated for MCP-CCP Protocol Specification Version 3.0 (replaced timestamps with sequence numbers)
%

classdef simple_mcp < handle

    properties (Access=public)
        serv_h
        br_list
        br_info
        seq_num % sequence number tracking
    end

    methods (Access=public)
        % constructor
        function obj = simple_mcp(varargin)

            p = inputParser;
            addOptional(p,'mcp_addr',"127.0.0.1");
            addOptional(p,'mcp_port',2000);
            parse(p,varargin{:});
            inargs = p.Results;

            % Initialize sequence number
            obj.seq_num = 1000;  % start from 1000 as recommended

            % create UDP socket
            obj.serv_h = udpport("datagram","LocalHost",inargs.mcp_addr,"LocalPort",inargs.mcp_port);
            configureCallback(obj.serv_h,"datagram",1,@obj.recvFcn)
            disp('MCP started, listening for connections')
        end

        % Increment sequence number
        function increment_seq_num(obj)
            obj.seq_num = obj.seq_num + 1;
        end

        % send a command to bladerunner
        function send_command(obj,BR_id,cmd)
            % search list of connected Bladerunners
            br_idx = 0;
            for ii = 1:length(obj.br_info)
                if strcmpi(obj.br_info{ii}.client_id,BR_id)
                    br_idx = ii;
                    break;
                end
            end

            if br_idx == 0
                fprintf(2,"Bladerunner %s not connected\n",BR_id);
            else
                cmdlist = {'FSLOWC','FFASTC','STOPC','STOPO','OPEN','CLOSE'};
                if ismember(upper(cmd),cmdlist)
                    msg = struct("client_type","ccp","message","EXEC",...
                        "client_id",BR_id,"sequence_number",obj.seq_num,...
                        "action",upper(cmd));
                    obj.sendmsg(msg,obj.br_info{br_idx}.addr,obj.br_info{br_idx}.port);
                    obj.increment_seq_num();
                    disp("Command sent.");
                else
                    fprintf(2,"Invalid command %s. Choose from FSLOWC, FFASTC, STOPC, STOPO, OPEN, CLOSE\n",cmd);
                end
            end
        end

        % send message as JSON and send
        function sendmsg(obj,msg,addr,port)
            msg = jsonencode(msg);
            write(obj.serv_h,msg,"char",addr,port);
        end

        % handles incoming JSON packets
        function recvFcn(obj,src,~)
            m = read(src,1,"char");
            fprintf(1,'\nMessage received by MCP:\n');
            disp(m)

            try
                msg = jsondecode(m.Data);     
                disp(jsonencode(msg,"PrettyPrint",true))

                switch msg.message
                    case 'CCIN' % client initiation
                        % add to list of connected BRs
                        br_idx = 0;
                        for ii = 1:length(obj.br_info)
                            if strcmpi(obj.br_info{ii}.client_id,msg.client_id)
                                br_idx = ii;
                                break;
                            end
                        end
                        if br_idx == 0
                            % add new BR
                            obj.br_info{end+1} = struct('client_id',msg.client_id,...
                                'addr',m.SenderAddress, ...
                                'port',m.SenderPort);
                        end

                        % send ACK with sequence number instead of timestamp
                        msg = struct("client_type","ccp","message","AKIN", ...
                            "client_id",msg.client_id,"sequence_number",obj.seq_num);
                        obj.sendmsg(msg,m.SenderAddress,m.SenderPort);
                        obj.increment_seq_num();

                    case 'STAT' % handle status messages
                        % additional handling for status messages
                end
            catch
                fprintf(1,'Message decode error\n');    
            end
        end
    end   
end
