clear; close all;

import = readmatrix('SWEEPNO.CSV');

% Test 1: 120R resistor on port A, asynchronous
% Newest Test: 120+75R resistor on port A, synchronous

%%
vref = import(:,2);
voltage = import(:,3);
cref = -import(:,4);
current = -import(:,5);

power = zeros(size(current,1),1);
for i = 1:size(current,1)
    power(i) = voltage(i)*current(i)/1000000;
end

%%

figure(1);

subplot(3,1,1);
plot(voltage, current, 'rx');
ylabel('I_{PV} (mA)');
xlabel('V_{PV} (mV)');

subplot(3,1,2);
plot(voltage, current.*voltage, 'rx');
ylabel('P_{PV} (W)');
xlabel('V_{PV} (mV)');

subplot(3,1,3);
plot(current, current.*voltage, 'rx');
ylabel('P_{PV} (W)');
xlabel('I_{PV} (mA)');




