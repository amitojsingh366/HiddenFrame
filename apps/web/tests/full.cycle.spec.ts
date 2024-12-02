import { test, expect } from '@playwright/test';
import path from 'path';
import { v4 as uuidv4 } from 'uuid';

test('should register, log in successfully with valid credentials and the decoded message should be correct', async ({ page }) => {
    await page.goto('http://localhost:3000/register/admin');

    const username = uuidv4();
    // Registration
    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'correctPassword');
    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');
    await expect(page).toHaveURL('http://localhost:3000/login');

    // Login
    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'correctPassword');
    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');
    await expect(page).toHaveURL('http://localhost:3000'); // Back to main page if login successful

    // Upload Image
    const fileInput = page.locator('input[name="file"]');
    const filePath = path.resolve('public/HF_logo_light.png');
    await fileInput.setInputFiles(filePath);
    await page.fill('input[name="message"]', 'Test message');
    await page.click('button[type="submit"]');

    await page.waitForSelector('[data-testid="secret-key"]');
    const secretKey = await page.getByTestId("secret-key").first().innerText();

    // Verify image is uploaded
    const uploadedImage = page.locator('img[src*="/static/"]').first();
    await expect(uploadedImage).toBeVisible();

    // Click the uploaded image
    await uploadedImage.click();

    // Enter key for decoding
    const keyInput = page.locator('input[name="key"]');
    await keyInput.fill(secretKey || "2");
    // Click Decode button
    const decodeButton = page.locator('button[type="submit"]');
    await decodeButton.click();

    // Verify the decoded message
    await expect(page.getByText('Test message')).toBeVisible();
});