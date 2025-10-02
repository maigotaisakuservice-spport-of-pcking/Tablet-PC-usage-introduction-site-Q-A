document.addEventListener('DOMContentLoaded', () => {
    const win11Btn = document.getElementById('win11-btn');
    const win10Btn = document.getElementById('win10-btn');
    const win11Content = document.getElementById('win11-content');
    const win10Content = document.getElementById('win10-content');

    // Make sure all elements exist before adding event listeners
    if (win11Btn && win10Btn && win11Content && win10Content) {
        win11Btn.addEventListener('click', () => {
            // Show Win11 content, hide Win10
            win11Content.style.display = 'block';
            win10Content.style.display = 'none';

            // Update active button style
            win11Btn.classList.add('active');
            win10Btn.classList.remove('active');
        });

        win10Btn.addEventListener('click', () => {
            // Show Win10 content, hide Win11
            win10Content.style.display = 'block';
            win11Content.style.display = 'none';

            // Update active button style
            win10Btn.classList.add('active');
            win11Btn.classList.remove('active');
        });
    }
});