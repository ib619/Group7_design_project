clear; close all;

import1 = readmatrix('BATCYCLE3.CSV');
length = size(import1, 1);

import2 = readmatrix('DIAGNOSE3.CSV');
import2 = import2(end-length:end, :);

%% 
V_1 = import1(:,2);
V_2 = import1(:,3);
V_3 = import1(:,4);

current_ref = import1(:,5);
current_measure = import1(:,6);

disc1 = import1(:,7);
disc2 = import1(:,8);
disc3 = import1(:,9);

SOC1 = import2(:,5);
SOC2 = import2(:,6);
SOC3 = import2(:,7);

%% 

figure(1);

subplot(3,1,1);
plot(V_1); hold on;
plot(V_2); hold on;
plot(V_3);
ylabel('Voltage (mV)');
xlabel('Time (s)');
legend('V_1','V_2','V_3');

subplot(3,1,2);
plot(current_ref); hold on;
plot(current_measure);
ylabel('Current (mA)');
xlabel('Time (s)');
legend('Reference Current','Measured Current');

subplot(3,1,3);
plot(SOC1); hold on;
plot(SOC2); hold on;
plot(SOC3);
ylabel('SOC (%)');
xlabel('Time (s)');
legend('SOC1','SOC2','SOC3');