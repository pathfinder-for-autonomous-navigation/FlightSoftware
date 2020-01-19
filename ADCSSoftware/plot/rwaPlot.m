function rwaPlot(file)

    data = csvread(file);
    
    figure
    
    subplot(3, 2, 1)
    hold on
    plot(data(:, 1), data(:, 2), '-r')
    plot(data(:, 1), data(:, 5), '-b')
    hold off
    grid on
    title('Reaction Wheel Zero Speed')
    xlabel('Time (ms)')
    ylabel('Speed')
    legend('Unfiltered', 'Filtered', 'Location', 'Southeast')
    subplot(3, 2, 2)
    hold on
    plot(data(:, 1), data(:, 8), '-r')
    plot(data(:, 1), data(:, 11), '-b')
    hold off
    grid on
    title('Reaction Wheel Zero Ramp')
    xlabel('Time (ms)')
    ylabel('Ramp')
    legend('Unfiltered', 'Filtered', 'Location', 'Southeast')
    
    subplot(3, 2, 3)
    hold on
    plot(data(:, 1), data(:, 3), '-r')
    plot(data(:, 1), data(:, 6), '-b')
    hold off
    grid on
    title('Reaction Wheel One Speed')
    xlabel('Time (ms)')
    ylabel('Speed')
    legend('Unfiltered', 'Filtered', 'Location', 'Southeast')
    subplot(3, 2, 4)
    hold on
    plot(data(:, 1), data(:, 9), '-r')
    plot(data(:, 1), data(:, 12), '-b')
    hold off
    grid on
    title('Reaction Wheel One Ramp')
    xlabel('Time (ms)')
    ylabel('Ramp')
    legend('Unfiltered', 'Filtered', 'Location', 'Southeast')
    
    subplot(3, 2, 5)
    hold on
    plot(data(:, 1), data(:, 4), '-r')
    plot(data(:, 1), data(:, 7), '-b')
    hold off
    grid on
    title('Reaction Wheel Two Speed')
    xlabel('Time (ms)')
    ylabel('Speed')
    legend('Unfiltered', 'Filtered', 'Location', 'Southeast')
    subplot(3, 2, 6)
    hold on
    plot(data(:, 1), data(:, 10), '-r')
    plot(data(:, 1), data(:, 13), '-b')
    hold off
    grid on
    title('Reaction Wheel Two Ramp')
    xlabel('Time (ms)')
    ylabel('Ramp')
    legend('Unfiltered', 'Filtered', 'Location', 'Southeast')

end

