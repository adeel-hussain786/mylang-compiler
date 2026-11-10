const SAMPLES = {
  basic: `int a = 10;
int b = 20;
int result = (a + b) * 2;
print(result);`,

  negative: `int positiveVal = 50;
int negativeVal = -20;
int result = positiveVal + negativeVal;
print(result);`,

  division: `int total = (10 + 20) * 2;
int quotient = total / 7;
print(total);
print(quotient);`,

  comparisons: `int x = 15;
int y = 20;

int isLess = x < y;
int isEqual = x == y;
int isNotEqual = x != y;
int isGreaterOrEqual = x >= 10;

print(isLess);
print(isEqual);
print(isNotEqual);
print(isGreaterOrEqual);`,

  full: `int a = 25;
int b = -5;
int multiplier = 2;

int calculated = (a + b) * multiplier;
calculated = calculated / 4;
int isPassing = calculated >= 10;

print(calculated);
print(isPassing);`
};

const PHASE_NAMES = {
  lexer: '1. Lexical Analysis (Tokens)',
  parser: '2. Syntax Analysis (AST)',
  semantic: '3. Semantic Analysis (Symbol Table)',
  llvm: '4. LLVM Intermediate Representation (IR)',
  codegen: '5. Native Code Generation & Output'
};

let latestCompilation = null;

const codeEditor = document.getElementById('codeEditor');
const lineNumbers = document.getElementById('lineNumbers');
const charCount = document.getElementById('charCount');
const lineCount = document.getElementById('lineCount');
const sampleSelect = document.getElementById('sampleSelect');
const runBtn = document.getElementById('runBtn');
const clearBtn = document.getElementById('clearBtn');
const clearConsoleBtn = document.getElementById('clearConsoleBtn');
const showOutputBtn = document.getElementById('showOutputBtn');
const consoleOutput = document.getElementById('consoleOutput');
const outputViewTitle = document.getElementById('outputViewTitle');
const selectedPhaseBadge = document.getElementById('selectedPhaseBadge');
const statusPill = document.querySelector('.status-pill');

const stageElements = {
  lexer: document.getElementById('stage-lexer'),
  parser: document.getElementById('stage-parser'),
  semantic: document.getElementById('stage-semantic'),
  llvm: document.getElementById('stage-llvm'),
  codegen: document.getElementById('stage-codegen')
};

function getCurrentTime() {
  const now = new Date();
  return now.toTimeString().split(' ')[0];
}

function logToConsole(message, type = 'info', isCodeBlock = false) {
  const entry = document.createElement('div');
  entry.className = `log-entry ${type}`;

  const timeSpan = document.createElement('span');
  timeSpan.className = 'log-time';
  timeSpan.textContent = `[${getCurrentTime()}]`;

  const msgSpan = document.createElement('span');
  msgSpan.className = isCodeBlock ? 'log-msg code-block' : 'log-msg';
  msgSpan.textContent = message;

  entry.appendChild(timeSpan);
  entry.appendChild(msgSpan);
  consoleOutput.appendChild(entry);

  consoleOutput.scrollTop = consoleOutput.scrollHeight;
}

function updateEditorStats() {
  const text = codeEditor.value;
  const lines = text.split('\n');
  const lineTotal = lines.length;

  lineCount.textContent = `${lineTotal} ${lineTotal === 1 ? 'line' : 'lines'}`;
  charCount.textContent = `${text.length} ${text.length === 1 ? 'char' : 'chars'}`;

  let numbersHtml = '';
  for (let i = 1; i <= lineTotal; i++) {
    numbersHtml += `${i}<br>`;
  }
  lineNumbers.innerHTML = numbersHtml;
}

codeEditor.addEventListener('scroll', () => {
  lineNumbers.scrollTop = codeEditor.scrollTop;
});

codeEditor.addEventListener('input', updateEditorStats);

codeEditor.addEventListener('keydown', (e) => {
  if (e.key === 'Tab') {
    e.preventDefault();
    const start = codeEditor.selectionStart;
    const end = codeEditor.selectionEnd;

    codeEditor.value = codeEditor.value.substring(0, start) + '    ' + codeEditor.value.substring(end);
    codeEditor.selectionStart = codeEditor.selectionEnd = start + 4;
    updateEditorStats();
  }
});

function loadSample(sampleKey) {
  if (SAMPLES[sampleKey]) {
    codeEditor.value = SAMPLES[sampleKey];
    updateEditorStats();
    logToConsole(`Loaded sample program: "${sampleSelect.options[sampleSelect.selectedIndex].text}"`, 'info');
  }
}

sampleSelect.addEventListener('change', (e) => {
  loadSample(e.target.value);
});

function inspectPhase(phaseKey) {
  Object.keys(stageElements).forEach(key => {
    const el = stageElements[key];
    if (el) {
      if (key === phaseKey) {
        el.classList.add('selected');
      } else {
        el.classList.remove('selected');
      }
    }
  });

  const phaseName = PHASE_NAMES[phaseKey] || phaseKey;
  if (outputViewTitle) {
    outputViewTitle.textContent = `Phase Inspector: ${phaseName}`;
  }

  consoleOutput.innerHTML = '';

  if (!latestCompilation || !latestCompilation.phases || !latestCompilation.phases[phaseKey]) {
    if (selectedPhaseBadge) {
      selectedPhaseBadge.textContent = 'Not Compiled Yet';
      selectedPhaseBadge.style.color = 'var(--text-muted)';
    }
    logToConsole(`No compilation data available for ${phaseName}.`, 'notice');
    logToConsole('Click "Run" to compile and generate actual phase outputs.', 'tip');
    return;
  }

  const phaseData = latestCompilation.phases[phaseKey];
  const phaseStatus = phaseData.status;

  if (selectedPhaseBadge) {
    selectedPhaseBadge.textContent = `Status: ${phaseStatus.toUpperCase()}`;
    if (phaseStatus === 'success') {
      selectedPhaseBadge.style.color = 'var(--accent-green)';
    } else if (phaseStatus === 'failed') {
      selectedPhaseBadge.style.color = 'var(--accent-red)';
    } else {
      selectedPhaseBadge.style.color = 'var(--text-muted)';
    }
  }

  logToConsole(`=== ACTUAL OUTPUT OF COMPILER PHASE: ${phaseName.toUpperCase()} ===`, 'info');

  if (phaseStatus === 'not_run') {
    logToConsole(phaseData.content, 'notice');
  } else if (phaseStatus === 'failed') {
    logToConsole('Diagnostics Encountered in this Phase:', 'error');
    logToConsole(phaseData.content, 'error', true);
  } else {
    logToConsole('Phase completed successfully:', 'success');
    logToConsole(phaseData.content, 'info', true);
  }
}

function showRunOutput() {
  Object.values(stageElements).forEach(el => {
    if (el) el.classList.remove('selected');
  });

  if (outputViewTitle) {
    outputViewTitle.textContent = 'Program Output';
  }
  if (selectedPhaseBadge) {
    selectedPhaseBadge.textContent = 'Stage: Final Run';
    selectedPhaseBadge.style.color = 'var(--accent-blue)';
  }

  consoleOutput.innerHTML = '';

  if (!latestCompilation) {
    logToConsole('Welcome to MyCompiler IDE for MyLang.', 'info');
    logToConsole('Enter code on the left and click "Run" to compile and execute.', 'tip');
    return;
  }

  if (latestCompilation.success) {
    logToConsole('Compilation, IR generation & native execution succeeded!', 'success');
    logToConsole('Program Execution Output:', 'tip');
    logToConsole(latestCompilation.output ? latestCompilation.output : '(Program executed with no output)', 'info', true);
    logToConsole('Click any stage above to inspect its exact phase output.', 'tip');
  } else {
    logToConsole('Compilation halted due to diagnostics:', 'error');
    logToConsole(latestCompilation.errors || 'Unknown compilation error', 'error', true);
    if (latestCompilation.failing_stage) {
      logToConsole(`Failing Stage: ${PHASE_NAMES[latestCompilation.failing_stage]}. Click that stage above for details.`, 'notice');
    }
  }
}

Object.keys(stageElements).forEach(phaseKey => {
  const el = stageElements[phaseKey];
  if (el) {
    el.addEventListener('click', () => {
      inspectPhase(phaseKey);
    });
  }
});

if (showOutputBtn) {
  showOutputBtn.addEventListener('click', showRunOutput);
}

runBtn.addEventListener('click', async () => {
  const code = codeEditor.value.trim();

  if (!code) {
    logToConsole('Notice: The editor is empty. Enter some MyLang code or choose a sample to run.', 'notice');
    return;
  }

  showRunOutput();
  const lines = code.split('\n').length;
  logToConsole(`Submitting ${code.length} characters across ${lines} lines to compiler backend...`, 'info');

  Object.values(stageElements).forEach(el => {
    if (el) el.className = 'stage-step';
  });

  try {
    const response = await fetch('/api/compile', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ source: code })
    });

    if (!response.ok) {
      throw new Error(`Server returned HTTP ${response.status}`);
    }

    const data = await response.json();
    latestCompilation = data;

    if (data.success) {
      if (statusPill) {
        statusPill.textContent = 'Backend Online';
        statusPill.className = 'status-pill status-online';
      }

      Object.values(stageElements).forEach(el => {
        if (el) el.classList.add('active');
      });

      showRunOutput();
    } else {
      if (statusPill) {
        statusPill.textContent = 'Compile Error';
        statusPill.className = 'status-pill status-offline';
      }

      if (data.phases) {
        Object.keys(data.phases).forEach(key => {
          const p = data.phases[key];
          const el = stageElements[key];
          if (el) {
            if (p.status === 'success') {
              el.classList.add('active');
            } else if (p.status === 'failed') {
              el.classList.add('error');
            }
          }
        });
      }

      showRunOutput();
    }

  } catch (err) {
    latestCompilation = null;
    if (statusPill) {
      statusPill.textContent = 'Server Offline';
      statusPill.className = 'status-pill status-offline';
    }

    logToConsole('Server Bridge Offline: Local server is not currently reachable.', 'notice');
    logToConsole('To execute code and inspect phases in the browser:', 'tip');
    logToConsole('  1. Open a terminal in the project directory.', 'tip');
    logToConsole('  2. Run: python server.py', 'tip');
    logToConsole('  3. Open: http://localhost:8080 in your browser.', 'tip');
  }
});

clearBtn.addEventListener('click', () => {
  codeEditor.value = '';
  updateEditorStats();
  latestCompilation = null;
  Object.values(stageElements).forEach(el => {
    if (el) el.className = 'stage-step';
  });
  showRunOutput();
  logToConsole('Editor and console cleared.', 'info');
});

clearConsoleBtn.addEventListener('click', () => {
  consoleOutput.innerHTML = '';
  logToConsole('Console output cleared.', 'info');
});

window.addEventListener('DOMContentLoaded', () => {
  loadSample('basic');
});
