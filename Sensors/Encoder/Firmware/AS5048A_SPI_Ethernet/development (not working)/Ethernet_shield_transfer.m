
instrreset;     % this closes all communication lines in matlab
t = tcpip('192.168.1.240', 30000, 'NetworkRole', 'client');
fopen(t);
fwrite(t,1);  % t is the object, 1 is the chip select 
disp('address');
%t.Terminator= 'CR/LF';
while(1)
    a = fread(t,1);
    j = fread(t,1);
    b = fread(t,1);
    disp(a+j*1000+255*b);
end
%fclose(t);
