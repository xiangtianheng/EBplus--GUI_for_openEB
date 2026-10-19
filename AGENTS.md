# Repository Guidance

## Commands

- Prefix shell commands with `rtk`. See `/Users/a0000/.codex/RTK.md`.

## Cross-Platform Development

Keep the upstream platform-neutral product separate from this fork's macOS
support layer. Do not add macOS packaging or dependency support back to
upstream `main` without an explicit decision to change this policy. The
fork's `main` intentionally includes macOS support.

- Upstream: `Ziqing7226/EBplus__more-than-gui-for-openeb`, branch `main`.
  This repository has no `master` branch.
- macOS fork: `xiangtianheng/EBplus--GUI_for_openEB`. Publish macOS work here
  and target its `main` branch for integration reviews.
- `origin` tracks upstream; `fork` tracks the macOS fork. Fetch upstream
  changes from `origin/main` and push adaptation branches to `fork`.

### Branch Roles

- `origin/main` contains platform-neutral product code and is the upstream
  target for generic features and fixes.
- `fork/main` contains upstream `main` plus the macOS Apple Silicon integration
  layer: platform-specific build configuration, dependency compatibility,
  packaging, and macOS-only tests.
- `develop-MacOS` is a retained integration branch; target it only when
  explicitly requested, not as the default destination for macOS work.
- `feature/<name>` branches from `origin/main` and targets upstream `main`.
- `macos/<name>` branches from `fork/main` and targets the fork's `main`.
- `codex/sync-main-<short-sha>` is a temporary integration branch used only to bring
  a specific upstream `main` revision into the fork's `main`.

### Daily Rules

- Keep GUI, algorithms, and product behavior platform-neutral whenever
  possible. Prefer a small macOS overlay over a long-lived source fork.
- Do not open a pull request carrying the macOS overlay to upstream `main`.
- For generic work that needs macOS or camera validation, validate it on a
  temporary branch based on `fork/main`, then create a clean upstream `main`
  PR containing only the generic commits.
- Keep macOS-only changes in `macos/*` branches and review them against
  `fork/main`.
- Never force-push, rebase, or directly merge into shared `fork/main` or
  `develop-MacOS`. Rebase is allowed only on an unshared local feature branch
  before its pull request is opened.

### Synchronizing Upstream Into The Fork

Use a reviewable synchronization pull request whenever upstream `main` updates need to
reach the macOS integration line:

1. Fetch upstream and fork, then create `codex/sync-main-<short-sha>` from the current
   `fork/main`.
2. Integrate the selected `origin/main` revision into that temporary branch.
3. Resolve conflicts by keeping generic behavior owned by upstream `main` and
   macOS build/package glue owned by the fork.
4. Run the relevant macOS build and test checks.
5. Push to `fork` and open a pull request from `codex/sync-main-<short-sha>`
   to the fork's `main` branch.

Do not automate a rebase of the shared branch. Future automation may create
the synchronization branch and pull request, but it must leave conflict
resolution, test review, and merge approval visible to maintainers.

### Preserving Upstream Behavior

Upstream has rewritten some history and removed older GUI and algorithm
modules. During synchronization, use the current upstream versions of generic
code and preserve only the macOS build/runtime overlay. Review the final diff
against `origin/main`; do not resurrect removed modules or reapply old audit
fixes just because they remain in the macOS branch's history.
