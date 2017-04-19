%% MATLAB communication with Arduino via serial comm
% Test out the frequency of communicaiton over SPI to the T-motor encoder
% board. This code should work without changing any of the firmware on the
% Arduino. 

clc
clear all
close all
%%

s = serial('/dev/cu.usbmodem1421','BaudRate',115200,'DataBits',8);
fopen(s);

num_motors = 1;
runTime = 20;
tocNow = 0;
angle = nan(1,0);
freq = nan(1,0);

% initialize the encoders by setting the zero position
for i = 1:num_motors
    % change the chip select pin by sending a '0', '1', or '2'
    fprintf(s, num2str(i-1));  
    
    % zero out the encoder at current position
    fprintf(s, 'z');      
end

tic
while (tocNow < runTime )
    
    for i = 1:num_motors
        % set the chip select pin
        fprintf(s, num2str(i-1));
        
        % read data from serial
        readData = fscanf(s,'%s')

        % check if the read string is empty
        if ~isempty(readData)
            angle(end+1,i) = str2num(readData);

            % write to serial: '0' for encoder 1, '1' for encoder 2, and '2' for encoder 3
            % fprintf(s, '0');  
        end
    end
    
    tocLast = tocNow;
    tocNow = toc;
    dt = tocNow - tocLast;
    freq(end+1) = 1/dt;
end

avgHz = mean(freq) 

% close the serial port
fclose(s);


% to close all serial ports
all_s = instrfind;
fclose(all_s);
