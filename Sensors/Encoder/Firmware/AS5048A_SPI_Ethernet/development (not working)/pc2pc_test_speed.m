% server
t = tcpip('169.254.173.248',30000,'NetworkRole','server')
fopen(t);
data = 255;
while(1)
fwrite(t,data);
end