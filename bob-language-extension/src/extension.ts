import * as vscode from 'vscode';

export function activate(context: vscode.ExtensionContext) {
    console.log('Bob language extension is now active!');

    // Register language configuration
    const bobLanguageConfig = vscode.languages.setLanguageConfiguration('bob', {
        comments: {
            lineComment: '//',
            blockComment: ['/*', '*/']
        },
        brackets: [
            ['{', '}'],
            ['[', ']'],
            ['(', ')']
        ],
        autoClosingPairs: [
            { open: '{', close: '}' },
            { open: '[', close: ']' },
            { open: '(', close: ')' },
            { open: '"', close: '"', notIn: ['string'] },
            { open: "'", close: "'", notIn: ['string'] }
        ],
        surroundingPairs: [
            ['{', '}'],
            ['[', ']'],
            ['(', ')'],
            ['"', '"'],
            ["'", "'"]
        ],
        indentationRules: {
            increaseIndentPattern: /\{[^}]*$|\b(func|if|else|while|for)\b.*$/,
            decreaseIndentPattern: /^\s*[})]/
        }
    });

    context.subscriptions.push(bobLanguageConfig);

    // Register hover provider for built-in functions
    const hoverProvider = vscode.languages.registerHoverProvider('bob', {
        provideHover(document, position, token) {
            const range = document.getWordRangeAtPosition(position);
            const word = document.getText(range);

            const builtinFunctions = {
                'print': 'Prints a value to the console',
                'assert': 'Asserts a condition and throws an error if false',
                'input': 'Gets user input from the console',
                'type': 'Returns the type of a value',
                'toString': 'Converts a value to a string',
                'toNumber': 'Converts a value to a number',
                'time': 'Returns the current time in microseconds'
            };

            if (builtinFunctions[word]) {
                return new vscode.Hover(`**${word}()** - ${builtinFunctions[word]}`);
            }

            return null;
        }
    });

    context.subscriptions.push(hoverProvider);

    // Register completion provider
    const completionProvider = vscode.languages.registerCompletionItemProvider('bob', {
        provideCompletionItems(document, position, token, context) {
            const completions = [];

            // Keywords
            const keywords = ['if', 'else', 'while', 'for', 'break', 'continue', 'return', 'var', 'func'];
            keywords.forEach(keyword => {
                const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
                item.detail = 'Bob keyword';
                completions.push(item);
            });

            // Built-in functions
            const builtins = ['print', 'assert', 'input', 'type', 'toString', 'toNumber', 'time'];
            builtins.forEach(func => {
                const item = new vscode.CompletionItem(func, vscode.CompletionItemKind.Function);
                item.detail = 'Built-in function';
                item.insertText = func + '()';
                completions.push(item);
            });

            // Constants
            const constants = ['true', 'false', 'none'];
            constants.forEach(constant => {
                const item = new vscode.CompletionItem(constant, vscode.CompletionItemKind.Constant);
                item.detail = 'Bob constant';
                completions.push(item);
            });

            return completions;
        }
    }, '.');

    context.subscriptions.push(completionProvider);
}

export function deactivate() {
    console.log('Bob language extension is now deactivated!');
} 