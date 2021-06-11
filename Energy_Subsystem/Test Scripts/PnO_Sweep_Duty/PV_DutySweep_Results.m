clear; close all;

import = readmatrix('4parallelcells_test4_home.CSV');

% Test 1: 120R resistor on port A, asynchronous
% Newest Test: 120+75R resistor on port A, synchronous

%%
pwm = import(:,2);
voltage = import(:,3);
% voltage = import(:,5);
current = -import(:,4);

power = zeros(size(current,1),1);
for i = 1:size(current,1)
    power(i) = voltage(i)*current(i)/1000000;
end

%%

figure(1);

subplot(2,1,1);
plot(voltage, current, 'bx','Linewidth', 2, 'MarkerSize', 8);
ylabel('I_{PV} (mA)');
xlabel('V_{PV} (mV)');
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

subplot(2,1,2);
plot(voltage, power, 'bx','Linewidth', 2, 'MarkerSize', 8);
ylabel('P_{PV} (W)');
xlabel('V_{PV} (mV)');
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

set(gca, 'FontName', 'Arial')
sgtitle('Characteristic Curves','FontName', 'Arial');

figure(2);

subplot(3,1,1);
plot(pwm*100, current, 'bx','Linewidth', 2, 'MarkerSize', 8);
xlabel('PWM (%)');
ylabel('I_{PV} (mA)');
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

subplot(3,1,2);
plot(pwm*100, voltage, 'bx','Linewidth', 2, 'MarkerSize', 8);
xlabel('PWM (%)');
ylabel('V_{PV} (mV)');
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

subplot(3,1,3);
plot(pwm*100, power, 'bx', 'Linewidth', 2, 'MarkerSize', 8);
xlabel('PWM (%)');
ylabel('P_{PV} (W)');
set(gca,'LineWidth',2)
set(gca,'FontSize',12)
set(gca, 'FontName', 'Arial')

set(gca, 'FontName', 'Arial')
sgtitle('I_{PV}, V_{PV}, P_{PV} against PWM','FontName', 'Arial');






