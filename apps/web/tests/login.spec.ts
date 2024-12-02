import { test, expect } from '@playwright/test';
import { v4 as uuidv4 } from 'uuid';


test('has title', async ({ page }) => {
    await page.goto('http://localhost:3000/login');

    await expect(page).toHaveTitle(/HiddenFrame/);
});

test('has login label', async ({ page }) => {
    await page.goto('http://localhost:3000/login');

    await expect(page.locator('h2')).toHaveText(/Login/);
});

test('should register and log in successfully with valid credentials', async ({ page }) => {
    await page.goto('http://localhost:3000/register/admin');

    const username = uuidv4();

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'correctPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');

    await expect(page).toHaveURL('http://localhost:3000/login');

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'correctPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');

    await expect(page).toHaveURL('http://localhost:3000'); // go back to main page if login successfull

});

test('should not log in successfully with invalid username', async ({ page }) => {
    await page.goto('http://localhost:3000/register/admin');

    const username = uuidv4();

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'correctPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');




    await expect(page).toHaveURL('http://localhost:3000/login');

    await page.fill('input[name="username"]', 'wrongUsername');
    await page.fill('input[name="password"]', 'correctPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');




    await expect(page).toHaveURL('http://localhost:3000/login'); // stay on login page if login unsuccessfull
});


test('should not log in successfully with invalid password', async ({ page }) => {
    await page.goto('http://localhost:3000/register/admin');

    const username = uuidv4();

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'correctPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');


    await expect(page).toHaveURL('http://localhost:3000/login');

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'wrongPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');

    await expect(page).toHaveURL('http://localhost:3000/login'); // stay on login page if login unsuccessfull
});

test('should not log in successfully with invalid credentials', async ({ page }) => {
    await page.goto('http://localhost:3000/register/admin');
    
    const username = uuidv4();

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'correctPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');


    await expect(page).toHaveURL('http://localhost:3000/login');

    await page.fill('input[name="username"]', 'wrongUsername');
    await page.fill('input[name="password"]', 'wrongPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');


    await expect(page).toHaveURL('http://localhost:3000/login'); // stay on login page if login unsuccessfull
});

test('should not log in succesfully if username or password is missing', async ({ page }) => {
    await page.goto('http://localhost:3000/register/admin');

    const username = uuidv4();

    await page.fill('input[name="username"]', username);
    await page.fill('input[name="password"]', 'correctPassword');

    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');


    await expect(page).toHaveURL('http://localhost:3000/login');

    await page.fill('input[name="password"]', 'somePassword');
    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');


    await expect(page).toHaveURL('http://localhost:3000/login') // stay on login page if login unsuccessfull

    await page.goto('http://localhost:3000/login');

    await page.fill('input[name="username"]', 'someUser');
    await page.fill('input[name="password"]', '');
    await page.click('button[type="submit"]');
    await page.waitForLoadState('networkidle');


    await expect(page).toHaveURL('http://localhost:3000/login') // stay on login page if login unsuccessfull
});










