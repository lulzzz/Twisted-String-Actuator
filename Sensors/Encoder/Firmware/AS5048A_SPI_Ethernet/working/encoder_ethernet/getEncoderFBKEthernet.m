instrreset;
t = tcpip('169.254.173.240', 30000, 'NetworkRole', 'client');
fopen(t);
fwrite(t,1);
tocNow = 0;
runTime = 5;
dt = nan(100,1);
i = 1;

tic
while(tocNow < runTime)
    a = fread(t,1);
    b = fread(t,1);
    c = fread(t,1);
    d = fread(t,1);
    e = fread(t,1);
    f = fread(t,1);
    val3 = a*100+b;
    val2 = c*100+d;
    val1 = e*100+f;
    %disp('val3');
    %disp(val3);
    %disp('val2');
    %disp(val2);
    %disp('val1');
    %disp(val1);
    dt(i) = toc - tocNow;
    tocNow = toc;
    i = i+1;
end

f = 1/mean(dt)