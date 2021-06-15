clear; close all;

% import = readmatrix('DIAGNOSE2.CSV');
load('DIAGNOSE2.mat');

%%

V_1 = import(:,2);
V_2 = import(:,3);
V_3 = import(:,4);

SOC1 = import(:,5);
SOC2 = import(:,6);
SOC3 = import(:,7);

figure(1);

subplot(2,1,1);
plot(V_1); hold on;
plot(V_2); hold on;
plot(V_3);
ylabel('Voltage (mV)');
xlabel('Time (s)');
legend('V_1','V_2','V_3');

subplot(2,1,2);
plot(SOC1); hold on;
plot(SOC2); hold on;
plot(SOC3);
ylabel('SOC (%)');
xlabel('Time (s)');
legend('SOC1','SOC2','SOC3');