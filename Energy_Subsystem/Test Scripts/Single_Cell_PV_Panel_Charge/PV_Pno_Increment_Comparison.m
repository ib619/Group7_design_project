clear; close all;

import1 = readmatrix('Test3_03_strict/MPPT_PVC.CSV');
import2 = readmatrix('Test4_05_strict/MPPT_PVC.CSV');
import3 = readmatrix('Test5_01_strict/MPPT_PVC.CSV');

% import1 = readmatrix('Test7_03_IC_Refined/MPPT_PVC.CSV');
% import2 = readmatrix('Test9_05_IC_Refined/MPPT_PVC.CSV');
% import3 = readmatrix('Test8_01_IC_Refined/MPPT_PVC.CSV');

import1 = import1(1:170,:);
import2 = import2(1:170,:);
import3 = import3(1:170,:);

% PV panel on port A, battery on port B

% Test 1: increments of 3%
% Test 2: increments of 5%
% Test 3: increments of 1%

%%
pwm_1 = import1(:,2);
v_b_1 = import1(:,3);
v_a_1 = import1(:,6);
current_1 = import1(:,5);

power_1 = zeros(size(current_1,1),1);
for i = 1:size(current_1,1)
    power_1(i) = v_a_1(i)*current_1(i)/1000000;
end

pwm_2 = import2(:,2);
v_b_2 = import2(:,3);
v_a_2 = import2(:,6);
current_2 = import2(:,5);

power_2 = zeros(size(current_2,1),1);
for i = 1:size(current_2,1)
    power_2(i) = v_a_2(i)*current_2(i)/1000000;
end

pwm_3 = import3(:,2);
v_b_3 = import3(:,3);
v_a_3 = import3(:,6);
current_3 = import3(:,5);

power_3 = zeros(size(current_3,1),1);
for i = 1:size(current_3,1)
    power_3(i) = v_a_3(i)*current_3(i)/1000000;
end

%%

figure(1);

subplot(2,1,1);
plot(pwm_3*100,'Linewidth', 2); hold on;
plot(pwm_1*100,'Linewidth', 2); hold on;
plot(pwm_2*100,'Linewidth', 2);
ylabel('PWM (%)');
xlabel('Time (s)');
legend('1%','3%','5%','location','best');
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

subplot(2,1,2);
plot(power_3,'Linewidth', 2); hold on;
plot(power_1,'Linewidth', 2); hold on;
plot(power_2,'Linewidth', 2);
ylabel('Power (W)');
xlabel('Time (s)');
legend('1%','3%','5%','location','best');
set(gca,'LineWidth',2);
set(gca,'FontSize',12);
set(gca, 'FontName', 'Arial')
sgtitle('P&O Increment Comparison','FontName', 'Arial');








