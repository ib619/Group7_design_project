close all; clear;

%% Load Results

import = readmatrix('CELL1TEST1.csv');

%% Seperate into columns

state = import(:,1);
voltage = import(:,2);
SOC = import(:,3);
current_ref = import(:,4);
current_measured = import(:,4);
length = size(import, 1);

%% Plot results
% each time interval is 1s. convenient!

figure(1);
subplot(4,1,1);
plot(state);
xlabel('Time(s)');
ylabel('State');
title('Battery Characteristic');

subplot(4,1,2);
plot(voltage);
xlabel('Time(s)');
ylabel('Voltage (mV)');

subplot(4,1,3);
plot(current_ref, 'r'); hold on;
plot(current_measured,'b');
xlabel('Time(s)');
ylabel('Current (mA)');
legend('Reference', 'Actual');

subplot(4,1,4);
plot(SOC); hold on;
xlabel('Time(s)');
ylabel('SOC (%)');
ylim([-20 120]);