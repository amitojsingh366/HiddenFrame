import { test, expect } from '@playwright/test';
import { v4 as uuidv4 } from 'uuid';

test('has title', async ({ page }) => {
    await page.goto('http://localhost:3000/register');

    await expect(page).toHaveTitle(/HiddenFrame/);
});

test('has register label', async ({ page }) => {
    await page.goto('http://localhost:3000/register');

    await expect(page.locator('h2')).toHaveText(/Register/);
});

test('should not register successfully, if the username already exists', async ({ page }) => {
    await page.goto('http://localhost:3000/register/admin');

    const username = uuidv4();

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'samePassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');

    await expect(page).toHaveURL('http://localhost:3000/login'); // go to login page if registration successful

    await page.goto('http://localhost:3000/register/admin');

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'samePassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');

    await expect(page).toHaveURL('http://localhost:3000/register/admin'); // stay on register page if registration unsuccessfull

});

test('should not register successfully with missing username or password', async ({ page }) => {
    await page.goto('http://localhost:3000/register/admin');

    const username = uuidv4();

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', '');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');

    await expect(page).toHaveURL('http://localhost:3000/register/admin'); // stay on register page if registration unsuccessfull

    await page.goto('http://localhost:3000/register/admin');

    await page.fill('input[name="username"]', '');
    await page.fill('input[name="password"]', 'password');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');

    await expect(page).toHaveURL('http://localhost:3000/register/admin'); // stay on register page if registration unsuccessfull

});
