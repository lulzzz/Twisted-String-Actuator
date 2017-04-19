%% Compression for 3, 4 digit numbers with 1 decimal place resolution. This
% format requires 5 byte packets for a 12 digit number.

% theory
numBytes = 5;
byte2bit  = 8;
maxNumber = 2^(numBytes*byte2bit);

% From Arduino
th1 = 20.3;
th2 = 128.1;
th3 = 177.2;

n0 = 1e11;
n1 = 1e9;
n2 = 1e5;
n3 = 1e1;

format long g

data_sent = n0 + n1*th1 + n2*th2 + n3*th3

% In Matlab
data_rec(1) = round((data_sent - n0)/n1, 1);
data_rec(2) = round((data_sent - n0)/n2 - data_rec(1)*1e4, 1);
data_rec(3) = round(((data_sent - n0) - data_rec(1)*1e9 - data_rec(2)*1e5)/10, 1)



