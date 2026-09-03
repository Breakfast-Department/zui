# ZUI Development Skills

## Engineering Philosophy

ZUI development prioritizes simplicity, correctness, consistency, maintainability, and performance. Every implementation should strengthen the existing architecture rather than introduce unnecessary complexity.

Prefer explicit and understandable C over clever abstractions. Prefer existing project patterns over introducing new patterns. Prefer a small coherent implementation over a large generalized system designed for hypothetical future requirements.

The agent should always understand the responsibility of the code before modifying it. Code should be written with the assumption that another developer will need to maintain, debug, extend, and understand it later.

A successful implementation is not simply code that compiles. It must fit naturally into the architecture and behavior of the rest of ZUI.



## Repository Awareness

Before implementing a non-trivial change, inspect the relevant parts of the repository.

Understand existing:

* directory structure;
* source files;
* public headers;
* internal headers;
* data structures;
* widget hierarchy;
* rendering flow;
* layout system;
* event handling;
* window lifecycle;
* resource management;
* platform backend;
* build configuration;
* examples;
* tests.

Search for existing implementations before creating new ones.

If functionality similar to the requested feature already exists, extend or reuse it instead of creating a parallel implementation.

Never assume that an API, structure, helper, field, macro, or subsystem exists without checking the repository.



## Change Scope

Implement only what is necessary for the requested task.

Do not combine unrelated work with a feature implementation.

Avoid opportunistic:

* refactoring;
* formatting changes;
* renaming;
* dependency updates;
* architectural rewrites;
* file reorganizations;
* API redesigns.

A feature request should not become an excuse to rewrite unrelated portions of ZUI.

If a broader architectural change is genuinely required, identify why the existing architecture cannot support the feature and keep the change as focused as possible.



## Clean C

Write idiomatic, readable C.

Prefer straightforward control flow and explicit state over complicated macro-based abstractions.

Use the language naturally:

```c
if (!window) {
    return NULL;
}
```

rather than hiding important behavior behind macros or unnecessary abstraction layers.

Avoid unnecessary:

* macros;
* casts;
* global state;
* function-pointer indirection;
* heap allocations;
* nested conditions;
* duplicated logic;
* deeply coupled structures.

C code should be easy to inspect with a debugger.



## Formatting and Style

Follow the formatting style already used by the repository.

Do not reformat unrelated code.

Maintain consistent:

* indentation;
* brace placement;
* pointer formatting;
* spacing;
* naming;
* line structure;
* declaration style.

Consistency with the surrounding code takes precedence over personal formatting preferences.



## Naming

Names must communicate intent.

Use descriptive names for meaningful state:

```c
float width;
float height;
float corner_radius;
float spacing;
ZuiWidget *child;
ZuiWindow *window;
```

Avoid meaningless names outside genuinely small scopes.

Names should follow the existing ZUI convention for:

* public functions;
* internal functions;
* structures;
* enums;
* constants;
* variables;
* callbacks;
* files.

Never introduce a second naming convention.



## Functions

Every function should have a clear responsibility.

A function should not simultaneously manage unrelated concerns such as:

```text
input processing
layout calculation
resource allocation
rendering
event dispatch
```

unless the architecture explicitly requires that combination.

Do not split code into tiny functions merely to reduce line count.

Create helpers when they represent meaningful reusable operations or make a complex responsibility easier to understand.

Prefer functions with predictable inputs, outputs, and side effects.



## Control Flow

Prefer early returns when they make failure paths easier to understand.

For example:

```c
if (!widget) {
    return;
}

if (!window) {
    return;
}
```

Avoid deeply nested control flow when the same logic can be expressed more clearly.

Do not use clever control-flow tricks that make debugging harder.



## Error Handling

Every operation that can fail must be treated according to its failure semantics.

This includes:

* memory allocation;
* resource creation;
* file loading;
* font loading;
* image loading;
* renderer initialization;
* platform initialization;
* object creation.

Do not dereference potentially NULL values.

Do not ignore meaningful failure results.

Do not silently continue after a failure when doing so can leave the system in an invalid state.

Error handling must preserve cleanup of resources already acquired.



## Initialization

Every object must enter a valid state after initialization.

Do not depend on undefined or accidental initialization.

Explicitly establish important defaults.

When introducing a new field, determine:

* its initial value;
* whether zero has semantic meaning;
* whether it is optional;
* who owns it;
* when it becomes valid;
* when it becomes invalid.

Avoid partially initialized objects escaping into the rest of the system.



## Partial Failure

Constructors and initialization functions must account for failure occurring after earlier resources have already been acquired.

For example:

```text
object allocation
→ renderer resource
→ texture
→ font
→ registration
```

If registration fails, earlier resources must still be cleaned up correctly.

Every initialization path should have a corresponding failure cleanup path.



## Memory Ownership

Ownership must always be understandable.

For every pointer stored by a structure, determine:

```text
Who allocated it?
Who owns it?
Who may modify it?
Who destroys it?
How long must it remain valid?
```

Never infer ownership merely from pointer storage.

Before changing parent/child relationships, inspect how destruction currently works.



## Lifetime

Think about object lifetime explicitly.

Important lifetimes include:

```text
application lifetime
window lifetime
widget lifetime
resource lifetime
frame lifetime
callback lifetime
```

Do not store references to temporary stack data where a longer lifetime is required.

Be especially careful with:

```c
void *user_data
```

Callbacks must never receive pointers that can become invalid before callback execution.



## Destruction

Every owned resource must have a deterministic destruction path.

Check destruction during:

* normal shutdown;
* constructor failure;
* window destruction;
* widget removal;
* renderer shutdown;
* platform shutdown;
* application termination.

Avoid double destruction.

Avoid destroying an object while another object still depends on it.



## Parent and Child Relationships

Widget trees require explicit lifecycle reasoning.

Before changing a parent/child relationship, determine whether:

* adding a child transfers ownership;
* removing a child destroys it;
* destroying a parent destroys its children;
* children can exist independently;
* children hold references to their parent;
* parent pointers remain valid during destruction.

Never assume ownership transfer.



## Strings

Do not assume string ownership semantics.

Determine whether an API:

* copies a string;
* stores the pointer;
* takes ownership;
* expects immutable storage.

Never retain a pointer to a local buffer beyond its lifetime.

Be careful with dynamically allocated strings and cleanup paths.



## Resource Management

Resources such as fonts, textures, images, shaders, buffers, and platform objects should have explicit lifetimes.

Avoid repeated resource creation.

Do not load files or create GPU resources every frame unless the architecture explicitly requires it.

Reuse resources where appropriate.

Do not introduce resource caches without considering:

* ownership;
* invalidation;
* lifetime;
* memory usage;
* cleanup.



## Widget Architecture

Widgets should remain responsible for their own behavior and state.

A widget may participate in:

* layout;
* rendering;
* input;
* state changes;
* callbacks;
* parent/child relationships.

However, platform-specific implementation details should not leak unnecessarily into widgets.

A button should not directly manage Wayland protocol objects.

A label should not manage the global event loop.

A panel should not independently initialize the renderer.



## Widget Lifecycle

When creating or modifying a widget, consider its complete lifecycle:

```text
create
→ initialize
→ configure
→ attach
→ layout
→ render
→ receive input
→ update
→ detach
→ destroy
```

Do not implement only the creation path while ignoring destruction or detachment.



## Layout System

Layout belongs to the layout system.

Do not duplicate layout algorithms inside individual widgets.

When modifying layout behavior, consider:

* available parent size;
* padding;
* spacing;
* child size;
* fill behavior;
* alignment;
* minimum size;
* maximum size if supported;
* nested containers;
* hidden children;
* zero dimensions;
* fractional values.

Layout must remain deterministic and predictable.



## Geometry

Always know which coordinate system a value belongs to.

Possible coordinate spaces include:

```text
window coordinates
content coordinates
widget-local coordinates
screen coordinates
pointer coordinates
rendering coordinates
```

Do not mix coordinate spaces without an explicit transformation.

When debugging geometry, trace the entire transformation chain.



## Rendering

Rendering code must respect the renderer's architecture.

Do not randomly manipulate global OpenGL state from individual widgets.

When modifying rendering, consider:

* shader state;
* texture state;
* blending;
* clipping;
* viewport;
* framebuffer;
* transformation;
* draw order;
* GPU resource lifetime.

A rendering operation must not leave the renderer in an unexpected state.



## OpenGL State

OpenGL is stateful.

Any change to rendering should consider whether it modifies:

```text
shader
texture
blend state
scissor state
framebuffer
viewport
vertex state
```

If the renderer depends on a particular state, restore or properly transition that state according to the renderer's existing design.

Do not fix a visual problem by blindly changing OpenGL state.



## Rendering Performance

Avoid unnecessary work inside the render loop.

Do not perform repeated:

* heap allocation;
* file access;
* string construction;
* image decoding;
* font loading;
* shader compilation;
* GPU resource creation.

Frame-time operations should remain predictable.

Correctness comes first, but obvious unnecessary work should not be introduced.



## Input

Input should flow through the established event architecture.

Conceptually:

```text
platform event
→ window
→ coordinate conversion
→ hit testing
→ target widget
→ widget state
→ callback
```

Do not bypass established event propagation without a clear reason.



## Hit Testing

Hit testing must follow actual widget geometry.

Do not hard-code interaction rectangles separately from rendering geometry.

When widgets move or resize, their interactive regions must remain synchronized with their visual regions.

Consider:

* nested widgets;
* overlapping widgets;
* visibility;
* enabled state;
* clipping;
* coordinate conversion.



## Event Callbacks

Callbacks must have predictable execution semantics.

Do not unexpectedly trigger callbacks during:

* object construction;
* destruction;
* property assignment;

unless that behavior is explicitly part of the existing design.

Callback user data must remain valid for as long as the callback can execute.



## Wayland Backend

Wayland-specific code should remain isolated inside the platform/backend layer.

Do not leak protocol details into generic widgets unless explicitly required.

When modifying Wayland behavior, understand:

* object lifetime;
* event dispatch;
* surface lifecycle;
* input seats;
* pointer events;
* keyboard events;
* frame callbacks;
* compositor interaction.

Do not destroy protocol objects based solely on apparent inactivity.



## Platform Separation

Platform-specific code should not spread throughout the toolkit.

If a feature requires platform-specific behavior, determine whether it belongs in:

```text
platform layer
window layer
renderer layer
widget layer
```

and place it at the lowest appropriate abstraction boundary.



## Build System

Changes to CMake or other build configuration must be deliberate.

When adding:

* source files;
* headers;
* libraries;
* compile definitions;
* compiler options;
* platform dependencies;

ensure the build system explicitly represents the dependency.

Do not rely on accidental transitive configuration.

After modifying build configuration, perform a fresh configuration when practical.



## Dependencies

Do not add dependencies without justification.

Before adding one, check whether the requirement can be satisfied through existing:

* ZUI code;
* C standard functionality;
* current dependencies;
* system APIs.

A dependency should solve a meaningful problem rather than merely make implementation convenient.



## Public API

Public APIs should be small, consistent, and intentional.

Before adding a public API, inspect existing APIs that solve similar problems.

Follow established:

* naming;
* parameter ordering;
* return conventions;
* ownership semantics;
* NULL semantics;
* error behavior.

Do not introduce a new API pattern for a single feature.



## API Stability

Treat existing public APIs as stable.

Before changing or removing one:

* search all usages;
* inspect examples;
* inspect documentation;
* inspect tests;
* consider source compatibility;
* consider behavioral compatibility.

Prefer additive changes when they can solve the problem without breaking existing applications.



## Internal APIs

Internal functionality should remain internal whenever possible.

If a helper is used only inside one source file, prefer internal linkage where appropriate.

Do not expose private implementation details merely because doing so makes one implementation easier.



## Abstraction

Create abstractions around real concepts.

Do not create abstractions around hypothetical requirements.

Good abstraction:

```text
shared widget geometry calculation
```

Unnecessary abstraction:

```text
generic universal UI behavior framework
```

for a feature used by one widget.

The number of abstractions should reflect actual architectural complexity.



## Duplication

Avoid meaningful duplication.

If two components contain the same non-trivial logic, determine whether it belongs in a shared abstraction.

However, do not aggressively deduplicate unrelated code merely because two functions currently look similar.

Premature abstraction can be worse than small duplication.



## Magic Numbers

Avoid unexplained constants when they represent meaningful design decisions.

Use existing project constants or design values where available.

Do not create constants for every trivial literal.

The goal is semantic clarity, not constant proliferation.



## Comments

Comments should explain intent, constraints, or reasoning that cannot be understood directly from the code.

Useful comments explain:

* why an unusual implementation exists;
* ownership;
* platform restrictions;
* mathematical reasoning;
* performance decisions;
* workarounds;
* lifecycle requirements.

Avoid comments that merely translate code into English.



## TODO

TODOs must be meaningful.

Avoid:

```c
// TODO: fix
```

Prefer explaining:

```c
// TODO: replace the temporary texture lifetime handling once
// shared renderer resource ownership is implemented.
```

Remove obsolete TODOs.



## Documentation Quality

Documentation must remain synchronized with implementation.

When behavior changes, determine whether the change requires updates to:

* README;
* API documentation;
* examples;
* architecture documentation;
* comments.

Never document functionality that does not exist.

Never invent APIs in examples.

Examples should compile against the actual project API.



## Documentation Style

Documentation should be practical and developer-oriented.

Explain behavior, not merely names.

When relevant, document:

```text
purpose
usage
ownership
lifetime
parameters
return behavior
failure behavior
constraints
examples
```

Do not bury important behavior inside vague prose.



## Examples

Examples are part of the developer experience.

Keep them:

* readable;
* minimal;
* realistic;
* consistent with the actual API;
* compilable whenever intended as executable examples.

Do not use internal APIs in public examples unless explicitly demonstrating internal development.



## Performance Philosophy

Do not optimize based on assumptions.

When performance matters:

```text
identify bottleneck
→ understand cause
→ make focused change
→ verify behavior
→ measure when possible
```

Do not add complicated caching, pooling, batching, or custom allocators without evidence that they are necessary.



## Allocation Discipline

Avoid allocations in high-frequency paths.

Especially review:

```text
render loop
event loop
layout passes
input handling
animation updates
```

when introducing dynamic memory operations.

Stack allocation is preferable for small temporary values when lifetime permits.



## Frame Rate Independence

Time-based behavior should use elapsed time rather than assuming a fixed frame rate.

Animation and movement should remain consistent across different refresh rates.

Do not implement behavior as:

```text
fixed movement per frame
```

when it represents a real-world rate.



## Visual Consistency

ZUI should maintain a coherent visual language.

When adding or modifying UI components, respect established:

* spacing;
* typography;
* colors;
* corner radii;
* sizing;
* interaction states;
* visual hierarchy.

Do not make a single widget visually inconsistent merely because it looks attractive in isolation.



## Accessibility and Usability

Interactive controls should remain understandable and usable.

Consider:

* readable text;
* adequate hit areas;
* visible interaction states;
* predictable behavior;
* clear hierarchy.

Do not sacrifice usability for decorative visual effects.



## Testing

Testing should match the risk of the change.

A rendering change should receive runtime or visual verification where possible.

A lifecycle change should exercise creation and destruction.

An input change should test actual interaction.

A build-system change should verify configuration and compilation.

A public API change should verify affected usages and examples.



## Compiler Warnings

Warnings should be understood rather than blindly suppressed.

Do not add casts or compiler flags solely to silence a warning.

If a warning is legitimate and unavoidable, follow the project's established suppression mechanism and document the reason when necessary.



## Undefined Behavior

Treat potential undefined behavior as a serious defect.

Pay particular attention to:

* out-of-bounds access;
* uninitialized values;
* invalid pointer lifetime;
* use-after-free;
* double-free;
* invalid casts;
* signed overflow;
* invalid shifts;
* incorrect format strings;
* invalid object access.

Code that "works on the current machine" is not sufficient justification.



## Debugging Method

Debug systematically.

Do not repeatedly modify unrelated code until the symptom disappears.

Identify:

```text
expected behavior
actual behavior
first point where they diverge
```

Then fix the earliest incorrect state.

For GUI problems, trace:

```text
platform
→ event
→ coordinates
→ widget
→ state
→ layout/rendering
```

rather than patching the final visual symptom.



## Debug Output

Temporary debug output should be targeted.

Avoid printing every frame or every widget operation unless explicitly debugging a high-frequency issue.

Remove temporary debugging output when the task is complete unless the output is intentionally part of the project.



## Security

Do not introduce unsafe C practices for convenience.

Pay particular attention to:

* buffer boundaries;
* string handling;
* format strings;
* file paths;
* resource loading;
* pointer lifetime;
* integer overflow.

Never use user-controlled data directly as a format string.



## Threading

Do not introduce threading without understanding the platform and renderer constraints.

GUI, Wayland, and OpenGL operations may have thread-affinity requirements.

Before introducing concurrency, identify:

* ownership thread;
* synchronization;
* resource lifetime;
* destruction thread;
* data sharing.

Avoid concurrency when it does not provide a meaningful benefit.



## Global State

Minimize mutable global state.

Before adding a global variable, determine whether the state can instead belong to:

* application context;
* window;
* renderer;
* platform context;
* widget;
* resource manager.

Global mutable state increases coupling and makes lifecycle reasoning harder.



## Configuration

Configuration should be explicit and predictable.

Do not introduce hidden behavior controlled by undocumented environment variables, filesystem state, or compiler-specific assumptions.

Follow the project's existing configuration mechanisms.



## Portability

Do not assume the development environment represents every supported environment.

Avoid hard-coded assumptions about:

* display dimensions;
* compositor behavior;
* GPU vendor;
* refresh rate;
* filesystem layout;
* current working directory;
* compiler extensions.

Platform-specific behavior must remain isolated and intentional.



## Resource Paths

Never use developer-specific absolute paths.

Avoid:

```text
/home/developer/project/...
```

or equivalent machine-specific paths.

Use project-relative or configured resource paths according to the project's established resource system.



## Generated Files

Determine whether a file is generated before modifying it.

If generated, modify the source/template/generator instead when appropriate.

Do not commit generated output unless the repository intentionally tracks it.



## File Organization

Files should have meaningful responsibilities.

Do not create giant miscellaneous files.

Avoid putting unrelated utilities into a generic file simply because it is convenient.

Likewise, do not split every small helper into a separate source file without a meaningful architectural boundary.



## Refactoring

Refactor when it directly improves correctness, maintainability, or the requested feature.

Avoid large unrelated refactors.

A refactor should preserve behavior unless behavior change is intentional.

When refactoring lifecycle or ownership code, verify all affected paths.



## Breaking Changes

Breaking changes require explicit justification.

Do not silently:

* rename public APIs;
* remove public APIs;
* change ownership semantics;
* change callback behavior;
* change default behavior;
* alter structure layout exposed publicly.

If a breaking change is necessary, identify the impact before implementing it.



## Agent Reasoning

The agent should reason from the repository outward.

The preferred process is:

```text
understand
→ inspect
→ identify existing pattern
→ design smallest coherent change
→ implement
→ build
→ test
→ review diff
→ document
```

Do not start by generating large amounts of code and then attempting to force the repository to accommodate it.



## Handling Ambiguity

Do not guess when ambiguity can cause an architectural or API-level mistake.

Ask the developer when:

* public API design is unclear;
* ownership semantics are ambiguous;
* a breaking change is required;
* multiple architectural approaches are equally plausible;
* visual behavior has materially different interpretations;
* a new dependency is unavoidable.

Do not ask questions that repository inspection can answer.



## Agent Communication

When reporting completed work, distinguish between what was:

* implemented;
* compiled;
* tested;
* runtime tested;
* visually verified;
* not verified.

Never claim runtime verification when only compilation was performed.

Never claim tests passed if they were not executed.



## Git Discipline

Keep commits and changes focused.

Do not mix unrelated changes.

Before committing, inspect the diff for:

* accidental files;
* debug output;
* unrelated formatting;
* unused includes;
* temporary code;
* generated artifacts;
* hard-coded paths;
* unintended API changes.

The final diff should tell a coherent story.



## Commit Messages

Commit messages should describe the actual change.

Prefer concise messages such as:

```text
feat: add panel alignment
fix: correct widget hit testing
refactor: simplify layout calculation
docs: update widget examples
test: add button interaction test
chore: update build configuration
```

Avoid vague messages such as:

```text
update
changes
fix stuff
final
misc
```



## Review Before Completion

Before declaring a task complete, review the implementation from multiple perspectives.

### Architecture

Does the implementation belong in the correct subsystem?

### C Quality

Is the code readable, explicit, and consistent?

### Memory

Are ownership and lifetime correct?

### API

Was unnecessary public API avoided?

### Rendering

Could renderer or GPU state be corrupted?

### Input

Could event propagation or hit testing break?

### Layout

Could the change create inconsistent geometry?

### Performance

Did the implementation introduce unnecessary repeated work?

### Platform

Is platform-specific logic isolated?

### Documentation

Does documentation reflect the actual behavior?

### Testing

Was the relevant behavior actually verified?

### Scope

Did the change modify anything unrelated?



## Completion Standard

A task is considered complete only when the implementation is coherent with the rest of ZUI.

The agent should leave behind code that another developer can:

* read;
* understand;
* compile;
* debug;
* extend;
* maintain.

The shortest implementation is not necessarily the best implementation.

The most abstract implementation is not necessarily the best implementation.

The best implementation is the **smallest clear, correct, maintainable implementation that naturally belongs in the existing ZUI architecture**.



## Non-Negotiable Engineering Rules

**Inspect before modifying.**

**Never invent existing APIs.**

**Never assume ownership.**

**Never ignore a possible lifetime problem.**

**Never hide errors with arbitrary casts.**

**Never introduce unrelated refactoring.**

**Never add dependencies without justification.**

**Never leak platform-specific implementation into unrelated layers.**

**Never perform unnecessary resource allocation inside high-frequency paths.**

**Never claim verification that did not happen.**

**Never silently break public behavior.**

**Never document functionality that does not exist.**

**Never use examples that do not correspond to the real API.**

**Never sacrifice architectural coherence for a quick local fix.**

**Prefer simple, explicit, maintainable C.**

**When an existing abstraction can solve the problem, use it.**

**When the existing architecture cannot solve the problem cleanly, extend the architecture deliberately rather than adding a workaround.**



## Final Principle

Every change to ZUI should answer three questions:

> **Does it work?**

> **Does it belong here?**

> **Will another developer understand why it was implemented this way?**

If the answer to all three is yes, the implementation is likely consistent with ZUI's engineering standards.
