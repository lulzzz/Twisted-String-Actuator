% Example MATLAB - Arduino Code
% Simon Kalouche
%
%% NUKE the Workspace
clc
clearvars -except a
close all

%% Setup Arduino
a = arduino('/dev/cu.usbmodem1411','uno','libraries','spi');        % first arduino (upload adioes.pde (or .ino) to arduino)
encoder = spidev(a,'D10','BitOrder','msbfirst','Mode',1);

% specify the read, write and write enable commands referring to the EEPROM instruction set
readCmd = bin2dec('0000 0011');
writeCmd = bin2dec('0000 0010');
writeEnable = bin2dec('0000 0110');

%Write enable the device
writeRead(encoder, writeEnable);

readAddress = hex2dec('3fff');
writeAddress = hex2dec('0016');

dataToWrite = [writeCmd,readAddress];
writeRead(encoder, dataToWrite)

dataToWrite2 = [readCmd,readAddress];
writeRead(encoder, dataToWrite2)




runTime = 100;

% setup figure
figure(1);
hold on

% initialize
tic                         % start timer
i = 1;
while( toc < runTime )
    tocNow(i) = toc;           % update time   
    
    enc(i) = analogRead(a,0);               % read FSR sensor value
    pwm2deg = round(360*(enc(i)/255));     % map FSR value to a pwm value
    
    
    % plot
    plot(tocNow(i), enc(i),'k.');
    %drawnow
    
    dt = diff(tocNow);
    i = i+1;
end

